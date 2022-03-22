package org.opencoroutine.framework;

import java.util.function.Function;

/**
 * @author ZhangZiCheng
 * @date 2022/3/22 18:03
 */
public class FiberApi {

    /**
     * 创建纤程，{@code acl_fiber_create}
     *
     * @param function 函数
     * @param param    调用函数的参数
     * @param size     纤程的大小
     */
    public static native Object crate(Function<Object, Object> function, Object param, int size);

    /**
     * 主动让出CPU给其它纤程，{@code acl_fiber_yield}
     */
    public static native void yield();

    /**
     * 获取当前运行的纤程，{@code acl_fiber_running}
     *
     * @return 如果没有正在运行的纤程将返回null
     */
    public static native Object runningFiber();

    /**
     * 获取指定纤程的id，{@code acl_fiber_id}
     *
     * @param fiber 指定的纤程对象
     * @return 纤程id
     */
    public static native int getId(Object fiber);

    /**
     * 获取关联的纤程状态，{@code acl_fiber_status}
     *
     * @param fiber 指定的纤程，如果为null，返回当前正在运行的纤程状态
     * @return 纤程状态
     */
    public static native int getStatus(Object fiber);

    /**
     * 通知指定的纤程退出，{@code acl_fiber_kill}
     *
     * @param fiber 指定的纤程，不能为null
     */
    public static native void exit(Object fiber);

    /**
     * 检查指定的纤程是否已经退出，{@code acl_fiber_killed}
     */
    public static native boolean isExited(Object fiber);

    /**
     * 让当前纤程休眠一段时间，底层使用时间轮，{@code acl_fiber_delay}
     *
     * @param milliseconds 睡眠的毫秒数
     * @return 唤醒后返回的剩余毫秒数
     */
    public static native long delay(long milliseconds);
}
