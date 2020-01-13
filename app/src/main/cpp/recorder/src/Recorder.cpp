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



int Recorder::prepare(){
    if(mMediaWriter != nullptr){
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

    int ret;
    mFrameQueue = new SafetyQueue<AVMediaData *>();
    LOGI("record param: dst %s, width %d, height %d",param->dstFile, param->width, param->height);
    int outputWidth = param->width;
    int outputHeight = param->height;
    mYuvConvert = new YUVConvertor();
    mYuvConvert->setInputParams(param->width, param->height, param->pixFormat);
    mYuvConvert->setCrop(param->cropX, param->cropY, param->cropWidth, param->cropHeight);
    mYuvConvert->setRotate(param->rotateDegree);
    mYuvConvert->setScale(param->scaleWidth, param->scaleHeight);
    //前置摄像头需要镜像
    mYuvConvert->setMirror(param->isFrontCamera);

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
    mMediaWriter->addEncodeOptions("preset", "ultrafast");
    mMediaWriter->setQuality(param->quality > 0 ? param->quality : 23);
    mMediaWriter->setMMaxBitRate(param->maxRate);
    mMediaWriter->setOutoutPath(param->dstFile);
    mMediaWriter->setOutputVideo(outputWidth, outputHeight, param->frameRate, videoFormat);
    mMediaWriter->setOutputAudio(param->sampleRate, param->channels, audioFormat);
    //前置摄像头需要转换
    mMediaWriter->setRotate(param->isFrontCamera ? "180" : nullptr);

    //准备
    ret = mMediaWriter->prepare();
    if(ret < 0){
        release();
    }
    return 0;
}

void Recorder::startRecord() {
    mMutex.lock();
    mAbortRequest = false;
    mStartRequest = true;
    mCond.signal();
    mMutex.unlock();

    if(mRecordThread == nullptr){
        mRecordThread = new Thread(this);
        mRecordThread->start();
        //子线程与主线程分离，子线程已结束，立即回收其资源
        mRecordThread->detach();
    }
}

void Recorder::stopRecord() {
    mMutex.lock();
    mAbortRequest = true;
    mCond.signal();
    mMutex.unlock();
    if(mRecordThread != nullptr){
        mRecordThread->join();
        delete mRecordThread;
        mRecordThread = nullptr;
    }
}

bool Recorder::isRecording() {
    bool recording = false;
    mMutex.lock();
    recording = !mAbortRequest && mStartRequest && !mExit;
    mMutex.unlock();
    return recording;
}

/**
 * 录制一帧数据
 */
int Recorder::recordFrame(AVMediaData *data) {
    if(mAbortRequest || mExit){
        LOGE("recorder is not recording");
        delete data;
        return -1;
    }

    //不允许音频录制 却有音频帧
    if(!mRecordParam->enableAudio && data->getType() == MediaAudio){
        delete data;
        return -1;
    }

    if(!mRecordParam->enableVideo && data->getType() == MediaVideo){
        delete data;
        return -1;
    }

    if(mFrameQueue != nullptr){
        mFrameQueue->push(data);
    } else{
        delete data;
        LOGE("frame queue is null");
        return -1;
    }

    return 0;
}

RecordParams* Recorder::getRecordParams() {
    return mRecordParam;
}

void Recorder::run() {
    int ret = 0;
    int64_t start = 0;
    int64_t current = 0;
    mExit = false;

    if(mRecordListener != nullptr){
        mRecordListener->onRecordStart();
    }
    LOGI("waiting to start record");
    while (!mStartRequest){
        if(mAbortRequest){
            break;
        }
        av_usleep(10000);
    }

    //开始录制解码流程
    if(!mAbortRequest && mStartRequest){
        LOGI("start record");
    }
    while (!mAbortRequest || !mFrameQueue->empty()){
        if(!mFrameQueue->empty()){
            auto data = mFrameQueue->pop();
            if(!data){
                continue;
            }
            if(start == 0){
                start = data->getPts();
            }
            if(data->getPts() >= current){
                current = data->getPts();
            }

            //yuv转码
            if(data->getType() == MediaVideo && mYuvConvert != nullptr){
                if(mYuvConvert->convert(data) < 0){
                    LOGE("failed to convert video data to yuv420p");
                    delete  data;
                    continue;
                }
            }

            //过滤
            if(mFrameFilter != nullptr &&  mFrameFilter->filterData(data) < 0){
                LOGE("failed to filter media data: %s", data->getName());
            }

            //编码
            ret = mMediaWriter->encodeMediaData(data);
            if(ret < 0){
                LOGE("Failed to encoder media data： %s", data->getName());
            } else{
                LOGI("recording time: %f", (float)(current - start));
                if (mRecordListener != nullptr) {
                    mRecordListener->onRecording((float)(current - start));
                }
            }

            //释放资源
            delete data;
        }
    }

    ret = mMediaWriter->stop();
    mExit = true;
    mCond.signal();
    if(mRecordListener != nullptr){
        mRecordListener->onRecordFinish(ret == 0, (float)(current - start));
    }
}