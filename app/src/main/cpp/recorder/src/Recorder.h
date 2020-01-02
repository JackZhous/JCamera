//
// Created by jackzhous on 2019/12/24.
//

#ifndef JCAMERA_RECORDER_H
#define JCAMERA_RECORDER_H


#include <Thread.h>
#include "SafetyQueue.h"
#include "RecordParams.h"
#include <AVMediaData.h>
#include <YUVConvertor.h>
#include <AVFrameFilter.h>
#include <AVMediaWriter.h>
#include <AndroidLog.h>
#include <AVFormatter.h>

extern "C"{
    #include <libavformat/avformat.h>
    #include <libavfilter/avfilter.h>
    #include <libavutil/time.h>

};

class OnRecordLisenter {
public:

    virtual void onRecordStart() = 0;

    virtual void onRecording(float duration) = 0;

    virtual void onRecordFinish(bool success, float ) = 0;

    virtual void onRecordError(const char *msg) = 0;
};



class Recorder : public Runnable{
    public:
        Recorder();
        virtual ~Recorder();

        void setMRecordListener(OnRecordLisenter *mRecordListener);

        //准备录制器
        int prepare();

        //释放资源
        void release();

        //录制媒体资源
        int recordFrame(AVMediaData *data);

        //开始录制
        void startRecord();

        //停止录制
        void stopRecord();

        bool isRecording();

        void run() override ;

        RecordParams *getRecordParams();

        RecordParams *getMRecordParam() const;

private:
    Mutex mMutex;
    Condition mCond;
    Thread *mRecordThread;
    OnRecordLisenter * mRecordListener;
    SafetyQueue<AVMediaData *> *mFrameQueue;
    bool mAbortRequest;
    bool mStartRequest;
    bool mExit;

    RecordParams *mRecordParam;     //录制参数
    YUVConvertor *mYuvConvert;      //Yuv转换器
    AVFrameFilter *mFrameFilter;   //AVFrame处理
    AVMediaWriter *mMediaWriter;    //媒体写入文件
};


#endif //JCAMERA_RECORDER_H
