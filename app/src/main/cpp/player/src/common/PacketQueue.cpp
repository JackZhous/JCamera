//
// Created by jackzhous on 2019/8/15.
//

#include "PacketQueue.h"

#include "unistd.h"

PacketQueue::PacketQueue() {
    reset();
}

PacketQueue::~PacketQueue() {
    flush();
    reset();
}


void PacketQueue::reset() {
    size = 0;
    pktLen = 0;
    duration = 0;
    abort = false;
    first = NULL;
    last = NULL;
}

void PacketQueue::flush() {
    PacketList* pkt;
    mMutex.lock();
    while (first != NULL){
        pkt = first;
        first = first->next;
        av_packet_unref(&pkt->pkt);
        av_freep(&pkt);
    }
    first = NULL;
    last = NULL;
    mMutex.unlock();
    LOGI("PacketQueue消息队列结束");
}

int PacketQueue::getPacketQueueSize() const {
    return size;
}

int PacketQueue::pushPacket(AVPacket *pkt) {
    int ret = 0;
    mMutex.lock();
    ret = pushPackets(pkt);
    mCond.signal();
    mMutex.unlock();

    if(!ret){
        av_packet_unref(pkt);
    }
    return ret;
}

int PacketQueue::pushPackets(AVPacket* pkt) {
    PacketList* list;

    list = (PacketList*)av_malloc(sizeof(PacketList));
    if(list == NULL){
        return PLAYER_FAILED;
    }
    if(abort){
        LOGI("reject receive AVPacket");
        return PLAYER_FAILED;
    }
    list->pkt = *pkt;
    list->next = NULL;

    if(!last){
        last = list;
        first = last;
    } else{
        last->next = list;
        last = list;
    }
    last->next = NULL;
    pktLen++;
//    LOGI("push one video packet %d pid %d", pktLen, gettid());
    duration += last->pkt.duration;
    size = size + sizeof(PacketList) + sizeof(pkt);
    return PLAYER_OK;
}

int PacketQueue::pullPacket(AVPacket *pkt) {
    if(pkt == NULL){
        return PLAYER_FAILED;
    }
    PacketList* temp;
    mMutex.lock();
    for(;;){
        if(abort){
            LOGI("reject get AVPacket");
            mMutex.unlock();
            return PLAYER_FAILED;
        }
        if(first == NULL){
            mCond.wait(mMutex);
            continue;
        }
        *pkt = first->pkt;
        temp = first->next;
        av_free(first);
        first = temp;
        pktLen--;
        duration -= pkt->duration;
        size = size - sizeof(PacketList) - sizeof(pkt);
//        LOGI("pull one video packet %d pid %d", pktLen, gettid());
        if(first == NULL){
            last = NULL;
        }
        break;
    }
    mCond.signal();
    mMutex.unlock();
    return PLAYER_OK;
}

int PacketQueue::getPacketLen() const {
    return pktLen;
}

void PacketQueue::abortRequest() {
    mMutex.lock();
    abort = true;
    mCond.signal();
    mMutex.unlock();
}

int64_t PacketQueue::getDuration() const {
    return duration;
}

void PacketQueue::start() {
    mMutex.lock();
    abort = false;
    mCond.signal();
    mMutex.unlock();
}