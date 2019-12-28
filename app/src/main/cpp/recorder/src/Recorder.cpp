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
        LOGI("recorder prepare failed");
        return -1;
    }

    RecordParams *param = mRecordParam;


}