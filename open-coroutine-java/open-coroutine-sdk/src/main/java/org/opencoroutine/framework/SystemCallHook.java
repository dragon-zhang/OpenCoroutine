package org.opencoroutine.framework;

import org.scijava.nativelib.NativeLoader;

import java.io.IOException;

/**
 * @author HaiLang
 * @date 2022/3/26 14:20
 */
public class SystemCallHook {

    private final static String LIB_NAME = "JniLibrary";

    static {
        try {
            NativeLoader.loadLibrary(LIB_NAME);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void hook(boolean enable) {
        SystemCallHookApi.hook(enable);
    }
}
