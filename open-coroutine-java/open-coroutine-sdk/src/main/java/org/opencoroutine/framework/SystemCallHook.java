package org.opencoroutine.framework;

/**
 * @author HaiLang
 * @date 2022/3/26 14:20
 */
public class SystemCallHook extends NativeLibraryLoader {
    public static void hook(boolean enable) {
        SystemCallHookApi.hook(enable);
    }
}
