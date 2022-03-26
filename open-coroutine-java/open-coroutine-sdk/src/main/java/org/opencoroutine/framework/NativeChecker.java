package org.opencoroutine.framework;

/**
 * @author ZhangZiCheng
 * @date 2022/03/19
 */
public class NativeChecker extends NativeLibraryLoader{
    public static String check(String param) {
        return NativeCheckerApi.check(param);
    }
}
