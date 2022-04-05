#include <jni.h>
#include <jni_md.h>
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

#define FIBER_EVENT_KERNEL  0
#define FIBER_EVENT_POLL    1
#define FIBER_EVENT_SELECT  2
#define FIBER_EVENT_WMSG    3

static jmethodID callback;

/* 协程处理入口函数 */
static void fiber_main(ACL_FIBER *fiber, void *ctx) {
    JNIEnv *env = NULL;
    if (jvm->AttachCurrentThread(reinterpret_cast<void **>(&env), NULL) == JNI_OK) {
        jclass *clazz = static_cast<jclass *>(ctx);
        env->CallStaticVoidMethod(*clazz, callback);
        jvm->DetachCurrentThread();
    }
}

void *handle(void *param) {
    ACL_FIBER *fiber = acl_fiber_create(fiber_main, param, 2048);
    acl_fiber_schedule_with(FIBER_EVENT_KERNEL);
    return NULL;
}

JNIEXPORT void JNICALL Java_org_opencoroutine_framework_SchedulerApi_startScheduler
        (JNIEnv *env, jclass clazz, jint mode) {
    callback = env->GetStaticMethodID(clazz, "callback", "()V");
    env->CallStaticVoidMethod(clazz, callback);
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