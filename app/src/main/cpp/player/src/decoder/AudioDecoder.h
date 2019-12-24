//
// Created by jackzhous on 2019/8/15.
//

#ifndef MYPLAYER_AUDIODECODER_H
#define MYPLAYER_AUDIODECODER_H

#include "MediaDecoder.h"

class AudioDecoder : public MediaDecoder{

public:
    AudioDecoder(AVCodecContext* codecCtx, AVStream* stream, int streamIndex, PlayerStatus* status);
    ~AudioDecoder();
    int getAudioFrame(AVFrame* frame);

private:
    bool packetAgain;
    AVPacket* packet;
    //保存上一次的时间值，下一次如果时间丢失，按照此值计算
    int64_t last_pts;
    AVRational last_tb;

};

#endif //MYPLAYER_AUDIODECODER_H
