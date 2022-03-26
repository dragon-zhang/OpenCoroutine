package org.opencoroutine.framework;

import java.util.function.Function;

/**
 * @author ZhangZiCheng
 * @date 2022/3/19 22:25
 */
public class SimpleJni {
    public static void main(String[] args) throws Throwable {
        System.out.println(NativeChecker.check("test"));
        SystemCallHook.hook(true);
        Fiber.crate(new Function<Object, Object>() {
            @Override
            public Object apply(Object o) {
                System.out.println(o);
                return null;
            }
        }, 1);
        Scheduler.startScheduler();
        Thread.sleep(1000);
    }
}
