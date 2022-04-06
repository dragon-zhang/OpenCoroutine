package org.opencoroutine.framework;

import java.util.function.Function;

/**
 * @author ZhangZiCheng
 * @date 2022/3/22 18:03
 */
public class Fiber extends NativeLibraryLoader {

    /**
     * 最小128KB
     */
    public static final int DEFAULT_SIZE = 128 * 1024 * 1024;

    public static Object crate(Function<Object, Object> function, Object param) {
        return crate(function, param, DEFAULT_SIZE);
    }

    /**
     * 创建纤程，{@code acl_fiber_create}
     *
     * @param function 函数
     * @param param    调用函数的参数
     * @param size     纤程的大小
     */
    public static Object crate(Function<Object, Object> function, Object param, int size) {
        return FiberApi.crate(function, param, size);
    }

    /**
     * 主动让出CPU给其它纤程，{@code acl_fiber_yield}
     */
    public static void yield() {
        FiberApi.yield();
    }

    /**
     * 获取当前运行的纤程，{@code acl_fiber_running}
     *
     * @return 如果没有正在运行的纤程将返回null
     */
    public static Object runningFiber() {
        return FiberApi.runningFiber();
    }

    /**
     * 获取指定纤程的id，{@code acl_fiber_id}
     *
     * @param fiber 指定的纤程对象
     * @return 纤程id
     */
    public static int getId(Object fiber) {
        return FiberApi.getId(fiber);
    }

    /**
     * 获取关联的纤程状态，{@code acl_fiber_status}
     *
     * @param fiber 指定的纤程，如果为null，返回当前正在运行的纤程状态
     * @return 纤程状态
     */
    public static int getStatus(Object fiber) {
        return FiberApi.getStatus(fiber);
    }

    /**
     * 通知指定的纤程退出，{@code acl_fiber_kill}
     *
     * @param fiber 指定的纤程，不能为null
     */
    public static void exit(Object fiber) {
        FiberApi.exit(fiber);
    }

    /**
     * 检查指定的纤程是否已经退出，{@code acl_fiber_killed}
     */
    public static boolean isExited(Object fiber) {
        return FiberApi.isExited(fiber);
    }

    /**
     * 让当前纤程休眠一段时间，底层使用时间轮，{@code acl_fiber_delay}
     *
     * @param milliseconds 睡眠的毫秒数
     * @return 唤醒后返回的剩余毫秒数
     */
    public static long delay(long milliseconds) {
        return FiberApi.delay(milliseconds);
    }
}
