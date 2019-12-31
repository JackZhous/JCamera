//
// Created by jackzhous on 2019/12/24.
//

#include "Recorder.h"

Recorder::Recorder(){
    mRecordListener = nullptr;
    mAbortRequest = true;
    mStartRequest = false;
    mExit = true;
    mRecordThread = nullptr;
    mYuvConvert = nullptr;
    mFrameFilter = nullptr;
    mFrameQueue = nullptr;
    mMediaWriter = nullptr;
    av_register_all();
    avfilter_register_all();
    mRecordParam = new RecordParams();
}


Recorder::~Recorder() {
    release();
    if(mRecordParam != nullptr){
        delete mRecordParam;
        mRecordParam = nullptr;
    }
}

void Recorder::release() {
    stopRecord();

    mMutex.lock();
    while (!mExit){
        mCond.wait(mMutex);
    }
    mMutex.unlock();

    if(mRecordListener != nullptr){
        delete mRecordListener;
        mRecordListener = nullptr;
    }
    if(mRecordThread != nullptr){
        delete mRecordThread;
        mRecordThread = nullptr;
    }
    if(mYuvConvert != nullptr){
        delete mYuvConvert;
        mYuvConvert = nullptr;
    }
    if(mFrameFilter != nullptr){
        delete mFrameFilter;
        mFrameFilter = nullptr;
    }
    if(mFrameQueue != nullptr){
        delete mFrameQueue;
        mFrameQueue = nullptr;
    }
    if(mMediaWriter != nullptr){
        delete mMediaWriter;
        mMediaWriter = nullptr;
    }
}

void Recorder::setMRecordListener(OnRecordLisenter *listener) {
    mRecordListener = listener;
}

RecordParams *Recorder::getMRecordParam() const {
    return mRecordParam;
}


int Recorder::prepare(){
    if(mMediaWriter == nullptr){
        LOGE("recorder prepare failed");
        return -1;
    }

    RecordParams *param = mRecordParam;
    if(param->rotateDegree % 90 != 0){
        LOGW("invalid rotate degree: %d", param->rotateDegree);
        return -1;
    }

    AVPixelFormat  videoFormat = getPixelFormat((PixelFormat)param->pixFormat);
    AVSampleFormat audioFormat = getSampleFormate((SampleFormat)param->sampleFormat);
    if((!param->enableVideo && !param->enableAudio)
                    || (param->enableVideo && videoFormat == AV_PIX_FMT_NONE)
                    && (param->enableAudio && audioFormat == AV_SAMPLE_FMT_NONE)){
        LOGE("pixel format and sample format invalid: %d, %d", param->pixFormat, param->sampleFormat);
        return -1;
    }

    int ret = -1;
    mFrameQueue = new SafetyQueue<AVMediaData *>();
    LOGI("record param: dst %s, width %d, height %d",param->dstFile, param->width, param->height);
    int outputWidth = param->width;
    int outputHeight = param->height;
    mYuvConvert = new YUVConvertor();
    mYuvConvert->setInputParams(param->width, param->height, param->pixFormat);
    mYuvConvert->setCrop(param->cropX, param->cropY, param->cropWidth, param->cropHeight);
    mYuvConvert->setRotate(param->rotateDegree);
    mYuvConvert->setScale(param->scaleWidth, param->scaleHeight);
    mYuvConvert->setMirror(param->mirror);

    if(mYuvConvert->prepare() < 0){
        delete mYuvConvert;
        mYuvConvert = nullptr;
    }else{
        videoFormat = AV_PIX_FMT_YUV420P;
        outputWidth = mYuvConvert->getOutputWidth();
        outputHeight = mYuvConvert->getOutputHeight();
        if(outputHeight == 0 || outputWidth == 0){
            outputWidth = param->rotateDegree % 180 == 0 ? param->width : param->height;
            outputHeight = param->rotateDegree % 180 == 0 ? param->height : param->width;
        }
    }

    //filter
    if((param->videoFilter && strcmp(param->videoFilter, "null") != 0)
            || (param->audioFilter && strcmp(param->audioFilter, "anull") != 0)){
        mFrameFilter = new AVFrameFilter();
        mFrameFilter->setVideoInput(outputWidth, outputHeight, videoFormat, param->frameRate, param->videoFilter);
        mFrameFilter->setVideoOutput(AV_PIX_FMT_YUV420P);
        mFrameFilter->setAudioInput(param->sampleRate, param->channels, audioFormat, param->audioFilter);
        if((ret = mFrameFilter->initFilter()) < 0){
            delete mFrameFilter;
            mFrameFilter = nullptr;
        } else{
            videoFormat = AV_PIX_FMT_YUV420P;
        }
    }

    mMediaWriter = new AVMediaWriter();
    mMediaWriter->setMUseTimeStamp(true);
    mMediaWriter->addEncodeOptions("preset", "uktrafast");
    mMediaWriter->setQuality(param->quality > 0 ? param->quality : 23);
    mMediaWriter->setMMaxBitRate(param->maxRate);
    mMediaWriter->setOutoutPath(param->dstFile);
    mMediaWriter->setOutputVideo(outputWidth, outputHeight, param->frameRate, videoFormat);
    mMediaWriter->setOutputAudio(param->sampleRate, param->channels, audioFormat);

    //准备

}