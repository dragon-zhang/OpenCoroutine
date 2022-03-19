package org.opencoroutine.framework;

import org.scijava.nativelib.NativeLoader;

import java.io.IOException;

/**
 * @author ZhangZiCheng
 * @date 2022/03/19
 */
public class NativeChecker {

    private final static String LIB_NAME = "JniLibrary";

    static {
        try {
            NativeLoader.loadLibrary(LIB_NAME);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static String check(String param) {
        return NativeCheckerApi.check(param);
    }
}
