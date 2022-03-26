package org.opencoroutine.framework;

/**
 * @author ZhangZiCheng
 * @date 2022/3/19 22:25
 */
public class SimpleJni {
    public static void main(String[] args) {
        System.out.println(NativeChecker.check("test"));
        SystemCallHook.hook(true);
    }
}
