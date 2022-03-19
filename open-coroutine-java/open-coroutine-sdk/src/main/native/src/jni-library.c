#include <jni.h>
#include <jni_md.h>
#include "org_opencoroutine_framework_NativeChecker.h" // under target/native/javah/

JNIEXPORT jstring JNICALL Java_org_opencoroutine_framework_NativeChecker_check
        (JNIEnv *env, jclass clazz, jstring param) {
    return param;
}