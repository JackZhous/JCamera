//
// Created by jackzhous on 2019/12/27.
//

#include "RecordParams.h"

RecordParams::RecordParams() : dstFile(nullptr), width(0), height(0), frameRate(0), pixFormat(0),
                                maxRate(0), quality(0), enableAudio(false), enableVideo(false),
                                videoEncoder(nullptr), sampleRate(0), channels(0), sampleFormat(0),
                                audioEncoder(nullptr), cropX(0), cropY(0), cropWidth(0),
                                cropHeight(0), rotateDegree(0), scaleHeight(0), scaleWidth(0),
                                mirror(false), videoFilter(nullptr), audioFilter(nullptr){

}


RecordParams::~RecordParams() {
    if(dstFile != nullptr){
        av_freep(&dstFile);
    }

    if(videoFilter != nullptr){
        av_freep(&videoFilter);
    }
    if(audioFilter != nullptr){
        av_freep(&audioFilter);
    }
    if(videoEncoder != nullptr){
        av_freep(&videoEncoder);
    }
    if(audioEncoder != nullptr){
        av_freep(&audioEncoder);
    }
}


void RecordParams::setAudioEncoder(const char *coder) {
    audioEncoder = av_strdup(coder);
}

void RecordParams::setAudioFilter(const char *filter) {
    audioFilter = av_strdup(filter);
}


void RecordParams::setAudioParams(int sampleRate, int sampleFormat, int channel) {
    this->sampleFormat = sampleFormat;
    this->sampleRate = sampleRate;
    this->channels = channel;
    enableAudio = true;
}

void RecordParams::setCrop(int x, int y, int cropW, int cropH) {
    cropX = x;
    cropY = y;
    this->cropWidth = cropW;
    cropHeight = cropH;
}

void RecordParams::setMaxBitRate(int bitRate) {
    maxRate = bitRate;
}

void RecordParams::setMirror(bool mirror) {
    this->mirror = mirror;
}

void RecordParams::setOutput(const char *url) {
    dstFile = strdup(url);
}

void RecordParams::setQuality(int quality) {
    this->quality = quality;
}

void RecordParams::setRotate(int degree) {
    rotateDegree = degree;
}

void RecordParams::setScale(int scaleW, int scaleH) {
    scaleWidth = scaleW;
    scaleHeight = scaleH;
}

void RecordParams::setVideoParams(int width, int height, int frameRate, int pixFormat,
                                  int maxBitRate, int quality) {
    this->width = width;
    this->height = height;
    this->frameRate = frameRate;
    this->pixFormat = pixFormat;
    this->quality = quality;
    this->maxRate = maxBitRate;
    enableVideo = true;
}

void RecordParams::setVideoEncoder(const char *coder) {
    videoEncoder = av_strdup(coder);
}

void RecordParams::setVideoFilter(const char *filter) {
    videoFilter = av_strdup(filter);
}

