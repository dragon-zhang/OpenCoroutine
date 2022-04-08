#include <jni.h>
#include <jni_md.h>
#include <iostream>
#include <pthread.h>
#include "fiber/lib_fiber.h"
#include "org_opencoroutine_framework_SchedulerApi.h"

static JavaVM *jvm;

int init_jvm(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    jint result = vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_8);
    if (JNI_OK != result) {
        return JNI_FALSE;
    }
    jvm = vm;
    return JNI_OK;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    init_jvm(vm, reserved);
    return JNI_VERSION_1_8;
}

long getCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

#define FIBER_EVENT_KERNEL  0
#define FIBER_EVENT_POLL    1
#define FIBER_EVENT_SELECT  2
#define FIBER_EVENT_WMSG    3

static jmethodID callback;

struct Context {
    JNIEnv *env;
    jclass clazz;
    int index;
};

/* 协程处理入口函数 */
static void fiber_main(ACL_FIBER *fiber, void *ctx) {
    Context *context = static_cast<Context *>(ctx);
    std::string s = "from fiber";
    s.append(std::to_string(context->index));
    jstring param = context->env->NewStringUTF((char *) s.data());
    context->env->CallStaticVoidMethod(context->clazz, callback, param);

    acl_fiber_kill(fiber);
    printf("killed fiber%d\n", context->index);
}

/**
 * 延迟detach
 */
static void detach(ACL_FIBER *fiber, void *ctx) {
    acl_fiber_delay(1500);

    printf("detach current thread...\n");
    jvm->DetachCurrentThread();

    acl_fiber_kill(fiber);
    printf("killed detach fiber\n");
}

void *handle(void *param) {

    jclass clazz = *static_cast<jclass *>(param);
    JNIEnv *env = NULL;
    jint result = jvm->AttachCurrentThread(reinterpret_cast<void **>(&env), NULL);
    if (result != JNI_OK) {
        printf("thread attach current thread failed !\n");
        return NULL;
    }
    jstring p = env->NewStringUTF("from thread");
    env->CallStaticVoidMethod(clazz, callback, p);

    //fixme 这里超过2个协程就会报奇怪的错
    for (int i = 0; i < 2; ++i) {
        Context *context = new Context;
        context->env = env;
        context->clazz = clazz;
        context->index = i;
        acl_fiber_create(fiber_main, context, 128 * 1024 * 1024);
    }

    acl_fiber_create(detach, NULL, 128 * 1024 * 1024);

    acl_fiber_schedule_with(FIBER_EVENT_KERNEL);
    return NULL;
}

JNIEXPORT void JNICALL Java_org_opencoroutine_framework_SchedulerApi_startScheduler
        (JNIEnv *env, jclass clazz, jint mode) {
    callback = env->GetStaticMethodID(clazz, "callback", "(Ljava/lang/String;)V");
    pthread_t t;
    pthread_create(&t, NULL, handle, &clazz);
//    int event_mode = FIBER_EVENT_KERNEL;
//    switch (mode) {
//        case FIBER_EVENT_KERNEL:
//            event_mode = FIBER_EVENT_KERNEL;
//            break;
//        case FIBER_EVENT_POLL:
//            event_mode = FIBER_EVENT_POLL;
//            break;
//        case FIBER_EVENT_SELECT:
//            event_mode = FIBER_EVENT_SELECT;
//            break;
//        case FIBER_EVENT_WMSG:
//            event_mode = FIBER_EVENT_WMSG;
//            break;
//    }
//    acl_fiber_schedule_with(event_mode);
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