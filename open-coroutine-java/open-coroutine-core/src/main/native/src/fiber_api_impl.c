#include <stdio.h>
#include <jni.h>
#include <jni_md.h>
#include <jvmti.h>
#include "fiber/lib_fiber.h"
#include "org_opencoroutine_framework_FiberApi.h"

static jmethodID apply = NULL;

struct Caller {
    JNIEnv *env;
    jobject function;
    jobject param;
};

/* 协程处理入口函数 */
static

void fiber_main(ACL_FIBER *fiber, void *ctx) {
    Caller *caller = (Caller *) ctx;
    if (NULL == apply) {
        jclass functionClass = caller->env->FindClass("java/util/function/Function");
        apply = caller->env->GetMethodID(functionClass, "apply", "(Ljava/lang/Object;)Ljava/lang/Object");
    }
    //调用apply方法
    jobject result = caller->env->CallObjectMethod(caller->function, apply, caller->param);
}

JNIEXPORT jobject JNICALL Java_org_opencoroutine_framework_FiberApi_crate
        (JNIEnv *env, jclass clazz, jobject function, jobject param, jint size) {
    struct Caller c = {env, function, param};
    Caller *caller = &c;
    //fixme 这里想下怎么返回fiber指针
    ACL_FIBER *fiber = acl_fiber_create(fiber_main, caller, size);
    return NULL;
}

JNIEXPORT void JNICALL Java_org_opencoroutine_framework_FiberApi_yield
        (JNIEnv *env, jclass clazz) {
    acl_fiber_yield();
}

JNIEXPORT jobject JNICALL Java_org_opencoroutine_framework_FiberApi_runningFiber
        (JNIEnv *env, jclass clazz) {
    //fixme 这里想下怎么返回fiber指针
    ACL_FIBER *fiber = acl_fiber_running();
    return NULL;
}

ACL_FIBER *from_jobject(jobject fiber) {
    //fixme 这里有类型转换问题
    return NULL;
}

JNIEXPORT jint JNICALL Java_org_opencoroutine_framework_FiberApi_getId
        (JNIEnv *env, jclass clazz, jobject fiber) {
    return acl_fiber_id(from_jobject(fiber));
}

JNIEXPORT jint JNICALL Java_org_opencoroutine_framework_FiberApi_getStatus
        (JNIEnv *env, jclass clazz, jobject fiber) {
    return acl_fiber_status(from_jobject(fiber));
}

JNIEXPORT void JNICALL Java_org_opencoroutine_framework_FiberApi_exit
        (JNIEnv *env, jclass clazz, jobject fiber) {
    acl_fiber_kill(from_jobject(fiber));
}

JNIEXPORT jboolean JNICALL Java_org_opencoroutine_framework_FiberApi_isExited
        (JNIEnv *env, jclass clazz, jobject fiber) {
    return acl_fiber_killed(from_jobject(fiber));
}

JNIEXPORT jlong JNICALL Java_org_opencoroutine_framework_FiberApi_delay
        (JNIEnv *env, jclass clazz, jlong milliseconds) {
    return acl_fiber_delay(milliseconds);
}