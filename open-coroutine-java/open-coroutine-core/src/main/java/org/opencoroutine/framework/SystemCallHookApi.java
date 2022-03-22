package org.opencoroutine.framework;

/**
 * @author ZhangZiCheng
 * @date 2022/3/22 18:29
 */
public class SystemCallHookApi {
    /**
     * 设置系统调用是否应该被hook，默认不hook，{@code acl_fiber_hook_api}
     *
     * @param enable 如果需要hook系统调用，传true
     */
    public static native void hook(boolean enable);

    static {
        System.out.println(true ? 1 : 0);
    }
}
