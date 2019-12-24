//
// Created by jackzhous on 2019/8/15.
//

#ifndef MYPLAYER_MEDIADECODER_H
#define MYPLAYER_MEDIADECODER_H

#include "Thread.h"
#include "common/PacketQueue.h"
#include <common/PlayerStatus.h>

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

class MediaDecoder : public Runnable{

    public:
        MediaDecoder(AVCodecContext* ctx, AVStream* stream, int index, PlayerStatus* playerStatus);
        ~MediaDecoder();
        virtual void run();
        int getStreamIndex() const ;
        int pushAVPacket(AVPacket* packet);
        PacketQueue* getPacketQueue() const ;
        AVCodecContext* getCodecCtx() const ;
        AVStream* getStream();

        int getMemorySize();
        bool hasEnoughPackets();
        virtual void flush();
        virtual void stop();
        virtual void start();


    protected:
        int streamIndex;
        AVCodecContext* pCodecCtx;
        PacketQueue* queue;
        AVStream* stream;
        PlayerStatus* playerStatus;
        bool abort;                //停止
        Mutex mMutex;
        Condition mCond;
};

#endif //MYPLAYER_MEDIADECODER_H
