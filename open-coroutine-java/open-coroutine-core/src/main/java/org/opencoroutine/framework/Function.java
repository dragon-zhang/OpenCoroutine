package org.opencoroutine.framework;

/**
 * @author HaiLang
 * @date 2022/3/26 21:30
 */
public class Function<T, R> {
    public native R apply(T t);
}
