package org.opencoroutine.framework;

import okhttp3.ConnectionPool;
import okhttp3.MediaType;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;
import okhttp3.ResponseBody;

import java.io.IOException;
import java.util.Optional;
import java.util.concurrent.TimeUnit;

public class HttpUtil {

    private static OkHttpClient client;

    private static final String DEFAULT_MEDIA_TYPE = "application/json; charset=utf-8";

    /**
     * 单例模式  获取类实例
     *
     * @return client
     */
    public static OkHttpClient getInstance() {
        if (client == null) {
            synchronized (OkHttpClient.class) {
                if (client == null) {
                    client = new OkHttpClient.Builder()
                            .connectionPool(new ConnectionPool(16, 5, TimeUnit.MINUTES))
                            .connectTimeout(5, TimeUnit.SECONDS)
                            .readTimeout(10, TimeUnit.MINUTES)
                            .build();
                }
            }
        }
        return client;
    }

    public static ResponseBody doGet(String url) {
        try {
            Request request = new Request.Builder().url(url).build();
            // 创建一个请求
            Response response = getInstance().newCall(request).execute();
            ResponseBody body = response.body();
            if (body != null) {
                return body;
            }
            throw new RuntimeException("exception in OkHttpUtil,response body is null");
        } catch (Exception ex) {
            return null;
        }
    }

    public static String doPost(String url, String postBody, String mediaType) {
        try {
            MediaType createMediaType = MediaType.parse(mediaType == null ? DEFAULT_MEDIA_TYPE : mediaType);
            Request request = new Request.Builder()
                    .url(url)
                    .post(RequestBody.create(createMediaType, postBody))
                    .build();

            Response response = getInstance().newCall(request).execute();
            String result;
            ResponseBody body = response.body();
            if (body != null) {
                result = body.string();
            } else {
                throw new IOException("exception in OkHttpUtil,response body is null");
            }
            return result;
        } catch (Exception ex) {
            return "";
        }
    }

    public static void main(String[] args) {
        for (int i = 0; i < 10; i++) {
            Optional.ofNullable(doGet("http://127.0.0.1:8081/rust"))
                    .ifPresent(body -> {
                        try {
                            System.out.println(body.string());
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    });
        }
    }
}