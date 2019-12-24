//
// Created by jackzhous on 2019-09-19.
//

#ifndef MYPLAYER_AUDIORESAMPLER_H
#define MYPLAYER_AUDIORESAMPLER_H

#include <cstdint>
#include <decoder/AudioDecoder.h>
#include <sync/MediaSync.h>
#include <device/AudioDevice.h>
#include "common/PlayerStatus.h"


extern "C"{
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
};

/**
 * 音频参数
 */
 typedef struct {
     int freq;
     int channels;
     int64_t channel_layout;
     enum AVSampleFormat fmt;
     int frame_size;
     int bytes_per_sec;
 }AudioParams;

 /**
  * 音频重采样状态结构体
  */
  typedef struct AudioState{
      double audioClock;            //音频时钟
      double audio_diff_cum;
      double audio_diff_avg_coef;
      double audio_diff_threshold;
      int audio_diff_avg_count;
      int audio_hw_buf_size;        //记录GLES整个缓冲区的长度
      uint8_t *outputBuffer;        //输出缓冲区大小
      uint8_t *resampleBuffer;      //重采样大小
      short *soundTouchBUffer;      //SoundTouch缓冲
      unsigned int buffersize;      //缓冲大小
      unsigned int resampleSize;    //重采样大小
      int bufferIndex;
      int writeBufferSize;          //写入大小
      SwrContext* swrCtx;
      int64_t audio_callback_time;      //音频回调时间
      AudioParams audioParamsSrc;       //音频原始参数
      AudioParams audioParamsTarget;    //音频目标参数

  };


class AudioResampler{
    public:
        AudioResampler(PlayerStatus* status, AudioDecoder* audioDecoder, MediaSync* sync);
        ~AudioResampler();

        int setResampleParams(AudioDeviceSpec* spec, int64_t wanted_channel_layout);

        void pcmQueueCallback(uint8_t *stream, int len);

    private:
        int audioSynchronize(int nbSamples);

        int audioFrameResample();

private:
    PlayerStatus* playerStatus;
    MediaSync* sync;
    AVFrame *frame;
    AudioDecoder* decoder;
    AudioState* audioState;
//    SoundTouchWrapper *soundTouchWrapper;   // 变速变调处理

};

#endif //MYPLAYER_AUDIORESAMPLER_H
