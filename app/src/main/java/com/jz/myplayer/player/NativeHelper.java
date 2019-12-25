package com.jz.myplayer.player;

import android.view.Surface;

/**
 * @author jackzhous
 * @package com.jz.myplayer
 * @filename NativeHelper
 * date on 2019/8/8 5:24 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class NativeHelper {

    static {
        System.loadLibrary("player");
        System.loadLibrary("ffmpeg");
        native_init();
    }


    public static class Holder {
        private static NativeHelper instance;


        public static NativeHelper getNativeHelper(){
            if(instance == null){
                instance = new NativeHelper();
            }

            return instance;
        }
    }


    public static native void native_init();
    //为什么要传一个相同的对象使用弱引用，因为要把这个弱引用回调回来使用
//    public native void nativeSetConnect(JMediaPlayer player, Object weakReference);

    public native void setUrl(String url);

    public native void setVideoPath(String videoPath);
    //异步准备,初始化内部的一些变量和开启读包线程
    public native void preparePlayerAsyn();

    public native int getVideoWidth();

    public native int getVideoHeight();

    public native int getRorate();

    public native void seekRequest(float time);

    public native void setLoop(int loop);

    public native void setVideoSurface(Surface surface);

    public native void start();

    public native void onPause();

    public native void onResume();

    public native void onStop();

    public native void release();
}
