package com.jz.jcamera.controller;

import java.nio.ByteBuffer;

/**
 * 回调到主线程接口
 */
public interface PCallBack {

    void showFps(float fps);

    void takePhotoSuccess(ByteBuffer buffer, int width, int height);
}
