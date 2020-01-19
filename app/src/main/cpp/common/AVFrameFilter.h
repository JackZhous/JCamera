//
// Created by jackzhous on 2019/12/27.
//

#ifndef JCAMERA_AVFRAMEFILTER_H
#define JCAMERA_AVFRAMEFILTER_H


#include "AVMediaData.h"

extern "C"{
#include <libavutil/pixfmt.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavutil/imgutils.h>
#include <libavfilter/buffersink.h>
};

class AVFrameFilter {
public:
    AVFrameFilter();

    virtual ~AVFrameFilter();

    void release();

    void setVideoInput(int width, int height, AVPixelFormat pixelFormat, int frameRate,
                                const char* filter);

    void setVideoOutput(AVPixelFormat format);

    void setAudioInput(int sampleRate, int channels, AVSampleFormat format, const char* filter);

    int initFilter();

    int initVIdeoFilter();

    int initAudioFilter();


    int filterData(AVMediaData* data);



private:
    void freeFrame(AVFrame* frame);

    //音频音频转码
    int filterAudio(AVMediaData* data);

    //创建视频转码
    int filterVideo(AVMediaData* data);

private:
    int mWidth;
    int mHeight;
    int mFrameRate;
    AVPixelFormat  mInputPixelFormat;
    AVPixelFormat  mOutPixelFormat;
    const char* mVideoFilter;           //视频过滤器描述 如黑白特效"lutyuv='u=128:v=128'";
    bool mVideoEnable;

    AVFilterContext* mVideoBuffersinkCtx;    //AVFilter输出端
    AVFilterContext* mVideoBuffersrcCtx;    //AVFilter输入端
    AVFilterGraph* mVideoFilterGraph;       //不知道是啥

    int mInSampleRate;                      //输入采样率
    int mInChannels;
    int mOutSampleRate;
    int mOutChannels;
    AVSampleFormat  mInSampleFormat;
    AVSampleFormat  mOutSampleFormat;
    const char *mAudioFilter;
    bool mAudioEnable;

    AVFilterContext *mAudioBuffersinkCtx;
    AVFilterContext *mAudioBuffersrcCtx;
    AVFilterGraph *mAudioFilterGraph;
};


#endif //JCAMERA_AVFRAMEFILTER_H
