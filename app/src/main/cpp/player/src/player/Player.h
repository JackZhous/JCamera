//
// Created by jackzhous on 2019/8/13.
//

#ifndef MYPLAYER_PLAYER_H
#define MYPLAYER_PLAYER_H

#include <common/PlayerStatus.h>
#include <common/MessageQueue.h>
#include <common/PlayerStatus.h>
#include <common/Config.h>
#include <Thread.h>
#include <sync/MediaSync.h>
#include "decoder/AudioDecoder.h"
#include "sync/MediaSync.h"
#include "decoder/VideoDecoder.h"
#include <device/AudioDevice.h>
#include <device/VideoDevice.h>

extern "C"{
    #include <libavformat/avformat.h>
    #include <libavutil/time.h>
};
#include "common/FFmpegUtil.h"
#include "AudioResampler.h"


class Player : public Runnable{
    public:
        Player();
        ~Player();

        MessageQueue* getMessageQueue();
        void setVideoUrl(char* url);
        void setVideoPath(char* path);

        int getWidth();
        int getHeight();
        int getRorate();
        void startPlay();
        void pause();
        void onResume();
        void reset();
        void stop();
        void setVideoDevice(VideoDevice* device);
        void prepare();
        void seekVideo(float times);
        void setLoop(int loop);
        void pcmQueueCallback(uint8_t *stream, int len);

    private:
        int prepareDecoder(int streamIndex);

        int prepareFFmpeg();
        void init();
    // open an audio output device
        int openAudioDevice(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate);

    protected:
        void run() override ;

private:
    PlayerStatus* playerStatus;

    AudioDecoder* aDecoder;
    VideoDecoder* vDecoder;
    AVFormatContext* pFormatCtx;
    short lastPlayerStatus;
    Mutex mMutex;
    Condition mCond;
    MediaSync* sync;
    bool attchmentRequest;          //封面包
    int eof;                        //文件播放结束标志
    bool mExit;
    AudioDevice* aDevice;
    VideoDevice* vDevice;

    AudioResampler *audioResampler;

    Thread* readPacketThread;
};


#endif //MYPLAYER_PLAYER_H
