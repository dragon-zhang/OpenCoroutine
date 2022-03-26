package org.opencoroutine.framework;

/**
 * @author HaiLang
 * @date 2022/3/26 14:20
 */
public class SystemCallHook extends NativeLibraryLoader {

    /**
     * 设置系统调用是否应该被hook，默认不hook，{@code acl_fiber_hook_api}
     *
     * @param enable 如果需要hook系统调用，传true
     */
    public static void hook(boolean enable) {
        SystemCallHookApi.hook(enable);
    }
}
