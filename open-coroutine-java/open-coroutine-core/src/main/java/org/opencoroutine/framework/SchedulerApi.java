package org.opencoroutine.framework;

import reactor.netty.http.client.HttpClient;

/**
 * @author ZhangZiCheng
 * @date 2022/3/22 18:25
 */
public class SchedulerApi {

    private static final HttpClient CLIENT = HttpClient.create();

    public static void callback(String param) {
        System.out.println("JNI(" + param + ") called from " + Thread.currentThread().getName());
        Thread.currentThread().setUncaughtExceptionHandler((t, e) -> {
            System.out.println("UncaughtExceptionHandler");
            e.printStackTrace();
        });
        try {
            String body = CLIENT.get()
                    .uri("http://127.0.0.1:8081/rust")
                    .responseContent()
                    .aggregate()
                    .asString()
                    .block();
            if (null == body) {
                System.out.println("fail");
            } else {
                System.out.println(body);
            }
        } catch (Throwable t) {
            System.out.println("Throwable");
            t.printStackTrace();
        }
        System.out.println("finished");
    }

    /**
     * 使用指定的事件类型启动纤程调度过程，默认事件类型是 FIBER_EVENT_KERNEL。
     * {@code acl_fiber_schedule_with}
     * FIBER_EVENT_KERNEL对于不同的操作系统平台是不同的：
     * Linux：epoll
     * BSD：kqueue
     * Windows：iocp
     *
     * @param mode 事件类型
     */
    public static native void startScheduler(int mode);

    /**
     * 检查当前线程是否处于纤程调度状态，{@code acl_fiber_scheduled}
     *
     * @return 如果处于调度状态，则返回true
     */
    public static native boolean isScheduling();

    /**
     * 停止纤程调度进程，所有纤程都将停止，{@code acl_fiber_schedule_stop}
     */
    public static native void stopScheduler();

    /**
     * 在后台创建一个纤程以释放死纤程，{@code acl_fiber_check_timer}
     *
     * @param size 在每个回收过程中释放的最大纤程数
     */
    public static native void clean(int size);
}
