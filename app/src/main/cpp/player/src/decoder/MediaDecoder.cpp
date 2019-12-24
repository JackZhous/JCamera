//
// Created by jackzhous on 2019/8/15.
//

#include "MediaDecoder.h"

MediaDecoder::MediaDecoder(AVCodecContext *ctx, AVStream *stream, int index, PlayerStatus* playerStatus) {
    this->pCodecCtx = ctx;
    this->stream = stream;
    streamIndex = index;
    this->playerStatus = playerStatus;
    queue = new PacketQueue();
    abort = false;
}


MediaDecoder::~MediaDecoder() {
    if(queue){
        queue->flush();
        delete queue;
        queue = NULL;
    }

    if(pCodecCtx){
        avcodec_close(pCodecCtx);
        avcodec_free_context(&pCodecCtx);
        pCodecCtx = NULL;
    }
    playerStatus = NULL;
}

void MediaDecoder::run() {}

int MediaDecoder::getStreamIndex() const {
    return streamIndex;
}

int MediaDecoder::pushAVPacket(AVPacket *packet) {
    return queue->pushPacket(packet);
}


PacketQueue* MediaDecoder::getPacketQueue() const {
    return queue;
}

AVCodecContext* MediaDecoder::getCodecCtx() const {
    return pCodecCtx;
}

bool MediaDecoder::hasEnoughPackets() {
    AutoMutex lock(mMutex);
    return queue == NULL || abort || stream->disposition & AV_DISPOSITION_ATTACHED_PIC
            || queue->getPacketLen() > MIN_FRAMES && (!queue->getDuration() ||
            queue->getDuration() * av_q2d(stream->time_base));
}


int MediaDecoder::getMemorySize() {
    return queue == NULL ? 0 : queue->getPacketQueueSize();
}

void MediaDecoder::flush() {
    if(queue){
        queue->flush();
    }
    //刷新解码器缓冲区
    avcodec_flush_buffers(pCodecCtx);
}

AVStream* MediaDecoder::getStream() {
    return stream;
}

void MediaDecoder::stop() {
    mMutex.lock();
    abort = true;
    mCond.signal();
    mMutex.unlock();
    if(queue){
        queue->abortRequest();
    }
}

void MediaDecoder::start() {
    if(queue){
        queue->start();
    }
    mMutex.lock();
    abort = false;
    mCond.signal();
    mMutex.unlock();
}