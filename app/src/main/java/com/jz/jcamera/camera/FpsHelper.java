package com.jz.jcamera.camera;

import com.jz.jcamera.util.JLog;

/**
 * fps计算工具辅助类
 */
public class FpsHelper {

    //秒
    private static final int TIME_S = 1;
    //毫秒
    private static final int TIME_MS = TIME_S * 1000;
    private static final int TIME_2MS = 2 * TIME_MS;

    private int frameCount;
    private long frameUpdateTime;
    //每秒多少帧
    private float fps;

    public FpsHelper() {
        frameCount = 0;
        frameUpdateTime = 0;
    }

    public float getFps() {
        return fps;
    }


    public void caluteFps(){
        long currentTime = System.currentTimeMillis();
        if(frameUpdateTime == 0){
            frameUpdateTime = currentTime;
        }
        if(currentTime - frameUpdateTime > TIME_MS){
            fps = (frameCount*1000.0f / (currentTime - frameUpdateTime));
            frameUpdateTime = currentTime;
            frameCount = 0;
        }
        frameCount++;
    }
}
