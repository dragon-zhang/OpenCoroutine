#include <jni.h>
#include <jni_md.h>
#include "fiber/lib_fiber.h"
#include "org_opencoroutine_framework_SystemCallHookApi.h"

JNIEXPORT void JNICALL Java_org_opencoroutine_framework_SystemCallHookApi_hook
(JNIEnv *env, jclass clazz, jboolean jboolean) {
    acl_fiber_hook_api(jboolean ? 1 : 0);
}