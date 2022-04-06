package org.opencoroutine.framework;

/**
 * @author ZhangZiCheng
 * @date 2022/3/19 22:25
 */
public class SimpleJni {
    public static void main(String[] args) throws Throwable {
        System.out.println(Thread.currentThread().getName());
        SystemCallHook.hook(true);
        Scheduler.startScheduler();
        Thread.sleep(1000);
        Scheduler.stopScheduler();
    }
}
