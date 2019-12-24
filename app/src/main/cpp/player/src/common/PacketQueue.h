//
// Created by jackzhous on 2019/8/15.
//

#ifndef MYPLAYER_PACKETQUEUE_H
#define MYPLAYER_PACKETQUEUE_H

#include "Thread.h"
#include "Config.h"
#include "AndroidLog.h"

extern "C"{
#include <libavcodec/avcodec.h>
};
typedef struct PacketList{
    AVPacket pkt;
    PacketList* next;
}PacketList;

class PacketQueue{
public:
    PacketQueue();
    ~PacketQueue();
    void flush();
    int pushPacket(AVPacket* pkt);
    int pullPacket(AVPacket* pkt);
    int getPacketQueueSize() const ;
    int64_t getDuration() const ;
    int getPacketLen() const ;
    void reset();               //为什么有这个方法，因为滑动进度条需要重置缓冲区的数据
    void abortRequest();
    void start();

private:
    int pushPackets(AVPacket* pkt);

private:
    Mutex mMutex;
    Condition mCond;
    PacketList* first;
    PacketList* last;   //队首取，队尾添加
    int size;           //包尺寸
    int pktLen;         //包个数
    int64_t duration;
    bool abort;
};

#endif //MYPLAYER_PACKETQUEUE_H
