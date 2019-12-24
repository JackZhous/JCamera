//
// Created by jackzhous on 2019/8/27.
//

#ifndef MYPLAYER_AUDIODEVICE_H
#define MYPLAYER_AUDIODEVICE_H

#include <Thread.h>
#include "common/PlayerStatus.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C"{
#include <libavutil/samplefmt.h>
};

//音频PCM填充回调
typedef void (*AduioPCMCallback) (void *userdata, uint8_t *stream, int len);

typedef struct {
    int freq;               //采样率
    AVSampleFormat format;  //音频采样格式
    uint8_t channels;       //声道
    uint16_t samples;       //采样大小
    uint32_t size;          //缓存区大小
    AduioPCMCallback callback;  //音频回调
    void* userData;             //音频上下文
}AudioDeviceSpec;

class AudioDevice : public Runnable {
    public:
        AudioDevice();
        ~AudioDevice();

        int open(AudioDeviceSpec* input, AudioDeviceSpec* output);

        void run() override ;

        void start();

        void stop();

        void pause();

        void resume();

        void flush();
    private:
        // 转换成SL采样率
        SLuint32 getSLSampleRate(int sampleRate);

        //计算音量
        SLmillibel getAmplificationLevel(float volumeLevel);


    private:
            //引擎接口
            SLObjectItf slObject;
            SLEngineItf slEngine;

            //混音器
            SLObjectItf slOutputMixObject;

            //播放器对象
            SLObjectItf  slPlayerObject;
            SLPlayItf slPlayItf;
            SLVolumeItf slVolumeItf;

            //缓冲器队列接口
            SLAndroidSimpleBufferQueueItf  slBufferQueueItf;

            AudioDeviceSpec audioDeviceSpec;        //音频设备参数
            int bytes_per_frame;                    //一帧占多少字节,默认一帧就是一个采样点
            int milli_per_buffer;                   //一个缓冲区时长占多少
            int frames_per_buffer;                  //一个缓冲区有多少帧
            int bytes_per_buffer;                   //一个缓冲区大小
            uint8_t *buffer;                         //缓冲区
            size_t buffer_capacity;                 //缓冲区大小

            Mutex mMutex;
            Condition mCond;
            Thread *audioThread;
            int abortRequest;
            int pauseRequest;
            int flushRequest;

            bool updateVolume;
            float leftVolume;
            float rightVolume;
};

#endif //MYPLAYER_AUDIODEVICE_H
