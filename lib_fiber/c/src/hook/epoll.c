#include "stdafx.h"

#ifdef	HAS_EPOLL

#include "common.h"
#include "fiber/libfiber.h"
#include "event.h"
#include "fiber.h"
#include "hook.h"

/****************************************************************************/

static EPOLL_EVENT *epfd_alloc(void)
{
	EPOLL_EVENT *ee = mem_calloc(1, sizeof(EPOLL_EVENT));
	int  maxfd = open_limit(0);

	if (maxfd <= 0) {
		msg_fatal("%s(%d), %s: open_limit error %s",
			__FILE__, __LINE__, __FUNCTION__, last_serror());
	}

	++maxfd;
	ee->fds  = (EPOLL_CTX **) mem_malloc(maxfd * sizeof(EPOLL_CTX *));
	ee->nfds = maxfd;

	return ee;
}

static ARRAY     *__main_epfds = NULL;
static __thread ARRAY *__epfds = NULL;

static pthread_key_t  __once_key;
static pthread_once_t __once_control = PTHREAD_ONCE_INIT;

static void thread_free(void *ctx fiber_unused)
{
	size_t j;
	ITER iter;

	if (__epfds == NULL) {
		return;
	}

	if (__epfds == __main_epfds) {
		__main_epfds = NULL;
	}

	foreach(iter, __epfds) {
		EPOLL_EVENT *ee = (EPOLL_EVENT *) iter.data;

		for (j = 0; j < ee->nfds; j++) {
			if (ee->fds[j] != NULL) {
				mem_free(ee->fds[j]);
			}
		}

		if (ee->epfd >= 0 && (*sys_close)(ee->epfd) < 0) {
			fiber_save_errno(acl_fiber_last_error());
		}

		mem_free(ee->fds);
		mem_free(ee);
	}

	array_free(__epfds, NULL);
	__epfds = NULL;
}

static void main_thread_free(void)
{
	if (__main_epfds) {
		thread_free(__main_epfds);
		__main_epfds = NULL;
	}
}

static void thread_init(void)
{
	if (pthread_key_create(&__once_key, thread_free) != 0) {
		msg_fatal("%s(%d), %s: pthread_key_create error %s",
			__FILE__, __LINE__, __FUNCTION__, last_serror());
	}
}

static EPOLL_EVENT *epoll_event_create(int epfd)
{ 
	EPOLL_EVENT *ee = NULL;
	size_t i;

	/* using thread specific to store the epoll handles for each thread*/
	if (__epfds == NULL) {
		if (pthread_once(&__once_control, thread_init) != 0) {
			msg_fatal("%s(%d), %s: pthread_once error %s",
				__FILE__, __LINE__, __FUNCTION__,
				last_serror());
		}

		__epfds = array_create(5);
		if (__pthread_self() == main_thread_self()) {
			__main_epfds = __epfds;
			atexit(main_thread_free);
		} else if (pthread_setspecific(__once_key, __epfds) != 0) {
			msg_fatal("pthread_setspecific error!");
		}
	}

	ee = epfd_alloc();
	array_append(__epfds, ee);

	/* duplicate the current thread's epoll fd, so we can assosiate the
	 * connection handles with one epoll fd for the current thread, and
	 * use one epoll fd for each thread to handle all fds
	 */
	ee->epfd = dup(epfd);

	for (i = 0; i < ee->nfds; i++) {
		ee->fds[i] = NULL;
	}

	return ee;
}

static EPOLL_EVENT *epoll_event_find(int epfd)
{
	ITER iter;

	if (__epfds == NULL) {
		msg_error("%s(%d), %s: __epfds NULL",
			__FILE__, __LINE__, __FUNCTION__);
		return NULL;
	}

	foreach(iter, __epfds) {
		EPOLL_EVENT *ee = (EPOLL_EVENT *) iter.data;
		if (ee->epfd == epfd) {
			return ee;
		}
	}

	return NULL;
}

int epoll_event_close(int epfd)
{
	ITER iter;
	EPOLL_EVENT *ee = NULL;
	int pos = -1;
	size_t i;

	if (__epfds == NULL || epfd < 0) {
		return -1;
	}

	foreach(iter, __epfds) {
		EPOLL_EVENT *e = (EPOLL_EVENT *) iter.data;
		if (e->epfd == epfd) {
			ee  = e;
			pos = iter.i;
			break;
		}
	}

	if (ee == NULL) {
		return -1;
	}

	for (i = 0; i < ee->nfds; i++) {
		if (ee->fds[i] != NULL) {
			mem_free(ee->fds[i]);
		}
	}

	mem_free(ee->fds);
	mem_free(ee);
	array_delete(__epfds, pos, NULL);

	return (*sys_close)(epfd);
}

/****************************************************************************/

int epoll_create(int size fiber_unused)
{
	EPOLL_EVENT *ee;
	EVENT *ev;
	int    epfd;

	if (sys_epoll_create == NULL) {
		hook_once();
	}

	if (!var_hook_sys_api) {
		return sys_epoll_create ? (*sys_epoll_create)(size) : -1;
	}

	ev = fiber_io_event();

	/* get the current thread's epoll fd */

	epfd = event_handle(ev);
	if (epfd < 0) {
		msg_error("%s(%d), %s: invalid event_handle %d",
			__FILE__, __LINE__, __FUNCTION__, epfd);
		return epfd;
	}

	ee = epoll_event_create(epfd);
	return ee->epfd;
}

#ifdef EPOLL_CLOEXEC
int epoll_create1(int flags)
{
	int epfd = epoll_create(100);

	if (epfd == -1) {
		return -1;
	}
	if (flags & EPOLL_CLOEXEC) {
		(void) close_on_exec(epfd, 1);
	}
	return epfd;
}
#endif

static void read_callback(EVENT *ev fiber_unused, FILE_EVENT *fe)
{
	EPOLL_CTX  *epx = fe->epx;
	EPOLL_EVENT *ee = epx->ee;

	assert(ee);
	assert(epx->mask & EVENT_READ);

	if (ee->nready >= ee->maxevents) {
		return;
	}

	ee->events[ee->nready].events |= EPOLLIN;
	memcpy(&ee->events[ee->nready].data, &ee->fds[epx->fd]->data,
		sizeof(ee->fds[epx->fd]->data));
	if (!(ee->events[ee->nready].events & EPOLLOUT)) {
		ee->nready++;
	}
	SET_READABLE(fe);
}

static void write_callback(EVENT *ev fiber_unused, FILE_EVENT *fe)
{
	EPOLL_CTX  *epx = fe->epx;
	EPOLL_EVENT *ee = epx->ee;

	assert(ee);
	assert(epx->mask & EVENT_WRITE);
	if (ee->nready >= ee->maxevents) {
		return;
	}

	ee->events[ee->nready].events |= EPOLLOUT;
	memcpy(&ee->events[ee->nready].data, &ee->fds[epx->fd]->data,
		sizeof(ee->fds[epx->fd]->data));
	if (!(ee->events[ee->nready].events & EPOLLIN)) {
		ee->nready++;
	}
	SET_WRITABLE(fe);
}

static void epoll_ctl_add(EVENT *ev, EPOLL_EVENT *ee,
	struct epoll_event *event, int fd, int op)
{
	if (ee->fds[fd] == NULL) {
		ee->fds[fd] = (EPOLL_CTX *) mem_malloc(sizeof(EPOLL_CTX));
	}

	ee->fds[fd]->fd      = fd;
	ee->fds[fd]->op      = op;
	ee->fds[fd]->mask    = EVENT_NONE;
	ee->fds[fd]->rmask   = EVENT_NONE;
	ee->fds[fd]->ee      = ee;
	ee->fds[fd]->fe->epx = ee->fds[fd];

	memcpy(&ee->fds[fd]->data, &event->data, sizeof(event->data));

	if (event->events & EPOLLIN) {
		ee->fds[fd]->mask |= EVENT_READ;
		ee->fds[fd]->fe    = fiber_file_open_read(fd);
		event_add_read(ev, ee->fds[fd]->fe, read_callback);
		SET_READWAIT(ee->fds[fd]->fe);
	}
	if (event->events & EPOLLOUT) {
		ee->fds[fd]->mask |= EVENT_WRITE;
		ee->fds[fd]->fe    = fiber_file_open_write(fd);
		event_add_write(ev, ee->fds[fd]->fe, write_callback);
		SET_WRITEWAIT(ee->fds[fd]->fe);
	}
}

static void epoll_ctl_del(EVENT *ev, EPOLL_EVENT *ee, int fd)
{
	if (ee->fds[fd]->mask & EVENT_READ) {
		event_del_read(ev, ee->fds[fd]->fe);
		CLR_READWAIT(ee->fds[fd]->fe);
	}
	if (ee->fds[fd]->mask & EVENT_WRITE) {
		event_del_write(ev, ee->fds[fd]->fe);
		CLR_WRITEWAIT(ee->fds[fd]->fe);
	}

	ee->fds[fd]->fd      = -1;
	ee->fds[fd]->op      = 0;
	ee->fds[fd]->mask    = EVENT_NONE;
	ee->fds[fd]->rmask   = EVENT_NONE;
	ee->fds[fd]->fe->epx = NULL;
	ee->fds[fd]->fe      = NULL;
	memset(&ee->fds[fd]->data, 0, sizeof(ee->fds[fd]->data));

	mem_free(ee->fds[fd]);
	ee->fds[fd] = NULL;
}

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
	EPOLL_EVENT *ee;
	EVENT *ev;

	if (sys_epoll_ctl == NULL) {
		hook_once();
	}

	if (!var_hook_sys_api) {
		return sys_epoll_ctl ?  (*sys_epoll_ctl)(epfd, op, fd, event) : -1;
	}

	ee = epoll_event_find(epfd);
	if (ee == NULL) {
		msg_error("%s(%d), %s: not exist epfd=%d",
			__FILE__, __LINE__, __FUNCTION__, epfd);
		return -1;
	}

	ev = fiber_io_event();

	if (op == EPOLL_CTL_ADD || op == EPOLL_CTL_MOD) {
		epoll_ctl_add(ev, ee, event, fd, op);
	} else if (op != EPOLL_CTL_DEL) {
		msg_error("%s(%d), %s: invalid op %d, fd %d",
			__FILE__, __LINE__, __FUNCTION__, op, fd);
		return -1;
	} else if (ee->fds[fd] != NULL) {
		epoll_ctl_del(ev, ee, fd);
		CLR_READWAIT(ee->fds[fd]->fe);
		CLR_WRITEWAIT(ee->fds[fd]->fe);
	} else {
		msg_error("%s(%d), %s: invalid fd=%d",
			__FILE__, __LINE__, __FUNCTION__, fd);
		return -1;
	}

	return 0;
}

static void epoll_callback(EVENT *ev fiber_unused, EPOLL_EVENT *ee)
{
	fiber_io_dec();

	if (ee->fiber->status != FIBER_STATUS_READY) {
		acl_fiber_ready(ee->fiber);
	}
}

static void event_epoll_set(EVENT *ev, EPOLL_EVENT *ee, int timeout)
{
	int i;

	for (i = 0; i < ee->maxevents; i++) {
		ee->events[i].events = 0;
	}

	if (timeout >= 0) {
		ee->expire = event_get_stamp(ev) + timeout;
		if (ev->timeout < 0 || timeout < ev->timeout) {
			ev->timeout = timeout;
		}
	} else {
		ee->expire = -1;
	}
}

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	EVENT *ev;
	EPOLL_EVENT *ee;
	long long now;
	int old_timeout;

	if (sys_epoll_wait == NULL) {
		hook_once();
	}

	if (!var_hook_sys_api) {
		return sys_epoll_wait ?  (*sys_epoll_wait)(epfd, events, maxevents, timeout) : -1;
	}

	ev = fiber_io_event();
	if (ev == NULL) {
		msg_error("%s(%d), %s: EVENT NULL",
			__FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	ee = epoll_event_find(epfd);
	if (ee == NULL) {
		msg_error("%s(%d), %s: not exist epfd %d",
			__FILE__, __LINE__, __FUNCTION__, epfd);
		return -1;
	}

	ee->events    = events;
	ee->maxevents = maxevents;
	ee->nready    = 0;
	ee->fiber     = acl_fiber_running();
	ee->proc      = epoll_callback;

	old_timeout = ev->timeout;
	event_epoll_set(ev, ee, timeout);
	ev->waiter++;

	while (1) {
		ring_prepend(&ev->epoll_list, &ee->me);

		fiber_io_inc();

		ee->fiber->status = FIBER_STATUS_EPOLL_WAIT;
		acl_fiber_switch();
		ev->timeout = old_timeout;

		ev->timeout = -1;
		if (acl_fiber_killed(ee->fiber)) {
			ring_detach(&ee->me);
			msg_info("%s(%d), %s: fiber-%u was killed",
				__FILE__, __LINE__, __FUNCTION__,
				acl_fiber_id(ee->fiber));
			break;
		}
		if (ee->nready != 0 || timeout == 0) {
			break;
		}

		now = event_get_stamp(ev);
		if (ee->expire > 0 && now >= ee->expire) {
			break;
		}
	}

	ev->waiter--;
	return ee->nready;
}

#endif	// end HAS_EPOLL
