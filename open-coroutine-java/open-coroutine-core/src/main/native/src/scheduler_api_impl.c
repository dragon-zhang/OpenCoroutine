#include <jni.h>
#include <jni_md.h>
#include "fiber/lib_fiber.h"
#include "org_opencoroutine_framework_SchedulerApi.h"

#define FIBER_EVENT_KERNEL  0
#define FIBER_EVENT_POLL    1
#define FIBER_EVENT_SELECT  2
#define FIBER_EVENT_WMSG    3

JNIEXPORT void JNICALL Java_org_opencoroutine_framework_SchedulerApi_startScheduler
        (JNIEnv *env, jclass clazz, jint mode) {
    int event_mode = FIBER_EVENT_KERNEL;
    switch (mode) {
        case FIBER_EVENT_KERNEL:
            event_mode = FIBER_EVENT_KERNEL;
            break;
        case FIBER_EVENT_POLL:
            event_mode = FIBER_EVENT_POLL;
            break;
        case FIBER_EVENT_SELECT:
            event_mode = FIBER_EVENT_SELECT;
            break;
        case FIBER_EVENT_WMSG:
            event_mode = FIBER_EVENT_WMSG;
            break;
    }
    acl_fiber_schedule_with(event_mode);
}

JNIEXPORT jboolean JNICALL Java_org_opencoroutine_framework_SchedulerApi_isScheduling
        (JNIEnv *env, jclass clazz) {
    return acl_fiber_scheduled() != 0;
}

JNIEXPORT void JNICALL Java_org_opencoroutine_framework_SchedulerApi_stopScheduler
        (JNIEnv *env, jclass clazz) {
    acl_fiber_schedule_stop();
}

JNIEXPORT void JNICALL Java_org_opencoroutine_framework_SchedulerApi_clean
        (JNIEnv *env, jclass clazz, jint jint) {
    acl_fiber_check_timer(jint > 0 ? jint : 1);
}