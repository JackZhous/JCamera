package com.jz.jcamera.recorder;

import android.os.Handler;

import com.jz.jcamera.camera.CameraHelper;
import com.jz.jcamera.camera.CameraManager;
import com.jz.jcamera.controller.OnRecordListener;
import com.jz.jcamera.util.JLog;


/**
 * @author jackzhous
 * @package com.jz.jcamera.recorder
 * @filename FFmpegRecorder
 * date on 2019/12/24 3:27 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class FFmpegRecorder {

    static {
        System.loadLibrary("ffmpeg");
        System.loadLibrary("yuv");
        System.loadLibrary("recorder");
    }

    //native录像管理器引擎
    private long nativeHandler;

    native long nativeInit();

    public native void setOutputs(long pointer, String dstPath);        //设置输出文件
    native void _setVideoParams(long pointer, int width, int height, int frameRate,
                                    int pixFormat, long maxBitRate, int quality);   //设置参数
    native void _setRotate(long pointer, int rotate);
    native void _setAudioParams(long pointer, int sampleRate, int sampleFormat, int channels);
    native void _setAudioEncoder(long pointer, String encoder);
    native void _setVideoEncoder(long pointer, String encoder);
    native void _setAudioFilter(long pointer, String filter);
    native void _setVideoFilter(long pointer, String filter);
    native void _release(long pointer);
    native void _startRecord(long pointer);
    native void _stopRecord(long pointer);
    native void _setFrontCamera(long pointer);              //设置前置摄像头

    native void _setRecordListener(long pointer, OnRecordListener listener);

    //音频数据
    native void _recordAudioFrame(long pointer, byte[] data, int len);

    //视频数据
    native void _recordVideoFrame(long pointer, byte[] data, int len, int width, int height, int pixelFormat);

    private String dstUrl;



    public FFmpegRecorder() {
        nativeHandler = nativeInit();

    }


    protected void setRecordListener(OnRecordListener listener){
        _setRecordListener(nativeHandler, listener);
    }

    protected void setDstUrl(String dstUrl) {
        this.dstUrl = dstUrl;
        JLog.i(dstUrl);
        setOutputs(nativeHandler, dstUrl);
    }

    protected void setVideoParams(int width, int height, int frameRate,
                                  int pixFormat, long maxBitRate, int quality){
        _setVideoParams(nativeHandler, width, height, frameRate, pixFormat, maxBitRate, quality);
    }

    protected void setRotate(int rotate){
        _setRotate(nativeHandler, rotate);
    }

    protected void setAudioParams(int sampleRate, int sampleFormat, int channels){
        _setAudioParams(nativeHandler, sampleRate, sampleFormat, channels);
    }
    /**
     * 指定音频编码器名称
     * @param encoder
     */
    protected void setAudioEncoder(String encoder) {
        _setAudioEncoder(nativeHandler, encoder);
    }

    /**
     * 指定视频编码器名称
     * @param encoder
     */
    protected void setVideoEncoder(String encoder) {
        _setVideoEncoder(nativeHandler, encoder);
    }

    /**
     * 设置音频AVFilter描述
     * @param filter
     */
    protected void setAudioFilter(String filter) {
        _setAudioFilter(nativeHandler, filter);
    }

    /**
     * 设置视频AVFilter描述
     * @param filter
     */
    protected void setVideoFilter(String filter) {
        _setVideoFilter(nativeHandler, filter);
    }

    public void release(){
        if(nativeHandler != 0){
            _release(nativeHandler);
            nativeHandler = 0;
        }
    }

    public void startRecord(){
        _startRecord(nativeHandler);
    }

    public void stopRecord(){
        _stopRecord(nativeHandler);
    }

    public void recordAudio(byte[] data, int len){
        _recordAudioFrame(nativeHandler, data, len);
    }

    public void recordVIdeo(byte[] data, int len, int width, int height, int pixelFormat){
        _recordVideoFrame(nativeHandler, data, len, width, height, pixelFormat);
    }

    public void setFrontCamerEnable(){
        _setFrontCamera(nativeHandler);
    }
}
