//
// Created by jackzhous on 2019/12/27.
//

#include "AVFrameFilter.h"

#define RATIONAL_MAX 1000000
#define SAMPLE_SIZE 1024

AVFrameFilter::AVFrameFilter() {
    av_register_all();
    avfilter_register_all();

    mWidth = 0;
    mHeight = 0;
    mFrameRate = 0;
    mInputPixelFormat = AV_PIX_FMT_NONE;
    mOutPixelFormat = AV_PIX_FMT_NONE;
    mVideoFilter = "null";
    mVideoEnable = false;
    mVideoBuffersrcCtx = nullptr;
    mVideoBuffersinkCtx = nullptr;
    mVideoFilterGraph = nullptr;

    mInSampleFormat = AV_SAMPLE_FMT_NONE;
    mOutSampleFormat = AV_SAMPLE_FMT_NONE;
    mInSampleRate = 0;
    mInChannels = 0;
    mOutChannels = 0;
    mAudioFilter = "anull";
    mAudioBuffersinkCtx = nullptr;
    mAudioBuffersrcCtx = nullptr;
    mAudioFilterGraph = nullptr;
    mAudioEnable = false;
}

AVFrameFilter::~AVFrameFilter() {
    release();
}


void AVFrameFilter::release() {
    mVideoBuffersinkCtx = nullptr;
    mVideoBuffersrcCtx = nullptr;
    if(mVideoFilterGraph != nullptr){
        avfilter_graph_free(&mVideoFilterGraph);
        mVideoFilterGraph = nullptr;
    }

    mAudioBuffersrcCtx = nullptr;
    mAudioBuffersinkCtx = nullptr;
    if(mAudioFilterGraph != nullptr){
        avfilter_graph_free(&mAudioFilterGraph);
        mAudioFilterGraph = nullptr;
    }
}

void AVFrameFilter::setVideoInput(int width, int height, AVPixelFormat pixelFormat, int frameRate,
                                  const char *filter) {
    mWidth = width;
    mHeight = height;
    mInputPixelFormat = pixelFormat;
    mFrameRate = frameRate;
    if(mOutPixelFormat == AV_PIX_FMT_NONE){
        mOutPixelFormat = pixelFormat;
    }
    mVideoFilter = (filter == nullptr) ? "null", filter;
    mVideoEnable = true;
}


void AVFrameFilter::setVideoOutput(AVPixelFormat format) {
    mOutPixelFormat = format;
}

void AVFrameFilter::setAudioInput(int sampleRate, int channels, AVSampleFormat format,
                                  const char *filter) {
    mInSampleRate = sampleRate;
    mInChannels = channels;
    mInSampleFormat = format;
    if(mOutSampleFormat == AV_SAMPLE_FMT_NONE){
        mOutSampleFormat = format;
    }

    if(mOutSampleRate == 0){
        mOutSampleRate = sampleRate;
    }
    if(mOutChannels == 0){
        mOutChannels = channels;
    }

    mAudioFilter = (filter == nullptr) ? "anull" : filter;
    mAudioEnable = true;


}
