package com.jz.jcamera.recorder;

import android.text.TextUtils;

import com.jz.jcamera.controller.OnRecordListener;

/**
 * @author jackzhous
 * @package com.jz.jcamera.recorder
 * @filename FFRecordBulder
 * date on 2020/1/7 10:44 AM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class FFRecordBulder {
    private String dstUrl;          //输出文件位置
    private OnRecordListener listener;      //jni回调
    private int mWidth,mHeight;
    private int mFrameRate;     //帧率  默认30fps
    private int mPixelFormat;       //像素格式
    private int mRotate;            //旋转角度
    private long mMaxBitRate;       //最大比特率，当无法达到设置的quality时，quality会自动调整
    private int mQuality;
    private String mVideoEncoder;   // 指定视频编码器名称
    private String mVideoFilter;    // 视频AVFilter描述


    private int mSampleRate;
    private int mSampleFormat;
    private int mChannels;
    private String mAudioEncoder;   // 指定音频编码器名称
    private String mAudioFilter;    // 音频AVFilter描述

    public FFRecordBulder(String dstUrl) {
        this.dstUrl = dstUrl;

        mWidth = -1;
        mHeight = -1;
        mRotate = 0;
        mPixelFormat = -1;
        mFrameRate = -1;
        mMaxBitRate = -1;
        mQuality = 23;
        mVideoEncoder = null;
        mVideoFilter = "null";

        mSampleRate = -1;
        mSampleFormat = -1;
        mChannels = -1;
        mAudioEncoder = null;
        mAudioFilter = "anull";
    }

    public FFRecordBulder setListener(OnRecordListener listener) {
        this.listener = listener;
        return this;
    }

    public FFRecordBulder setWH(int width, int height){
        this.mWidth = width;
        this.mHeight = height;
        return this;
    }

    public FFRecordBulder setmFrameRate(int mFrameRate) {
        this.mFrameRate = mFrameRate;
        return this;
    }

    public FFRecordBulder setmMaxBitRate(long mMaxBitRate) {
        this.mMaxBitRate = mMaxBitRate;
        return this;
    }

    public FFRecordBulder setmQuality(int mQuality) {
        this.mQuality = mQuality;
        return this;
    }

    public FFRecordBulder setmPixelFormat(int mPixelFormat) {
        this.mPixelFormat = mPixelFormat;
        return this;
    }

    public FFRecordBulder setmRotate(int mRotate) {
        this.mRotate = mRotate;
        return this;
    }

    public FFRecordBulder setmAudioEncoder(String mAudioEncoder) {
        this.mAudioEncoder = mAudioEncoder;
        return this;
    }

    public FFRecordBulder setmAudioFilter(String mAudioFilter) {
        this.mAudioFilter = mAudioFilter;
        return this;
    }

    public FFRecordBulder setmVideoEncoder(String mVideoEncoder) {
        this.mVideoEncoder = mVideoEncoder;
        return this;
    }

    public FFRecordBulder setmVideoFilter(String mVideoFilter) {
        this.mVideoFilter = mVideoFilter;
        return this;
    }

    public FFRecordBulder setAudioParams(int sampleRate, int sampleFormat, int channels){
        mSampleRate = sampleRate;
        mSampleFormat = sampleFormat;
        mChannels = channels;
        return this;
    }

    public FFmpegRecorder build(){
        FFmpegRecorder recorder = new FFmpegRecorder();
        recorder.setDstUrl(dstUrl);
        recorder.setRecordListener(listener);
        recorder.setVideoParams(mWidth, mHeight, mFrameRate, mPixelFormat, mMaxBitRate, mQuality);
        recorder.setRotate(mRotate);
        recorder.setAudioParams(mSampleRate, mSampleFormat, mChannels);

        if (!TextUtils.isEmpty(mVideoEncoder)) {
            recorder.setVideoEncoder(mVideoEncoder);
        }
        if (!TextUtils.isEmpty(mAudioEncoder)) {
            recorder.setAudioEncoder(mAudioEncoder);
        }
        if (!mVideoFilter.equalsIgnoreCase("null")) {
            recorder.setVideoFilter(mVideoFilter);
        }
        if (!mAudioFilter.equalsIgnoreCase("anull")) {
            recorder.setAudioFilter(mAudioFilter);
        }
        return recorder;
    }
}
