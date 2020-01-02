//
// Created by jackzhous on 2019/12/27.
//

#include "AVMediaData.h"

AVMediaData::AVMediaData() : image(nullptr), length(0), sample(nullptr), sample_size(0),
                             width(0), height(0), pixelFormat(PIXEL_FORMAT_NONE),
                             pts(0), type(MediaNone){}

AVMediaData::~AVMediaData() {
    free();
}

void AVMediaData::free() {
    if(image != nullptr){
        delete image;
        image = nullptr;
    }

    if(sample != nullptr){
        delete sample;
        sample = nullptr;
    }

    length = 0;
    sample_size = 0;
}

void AVMediaData::setPts(int64_t pts) {
    this->pts = pts;
}

void AVMediaData::setAudio(uint8_t *data, int size) {
    sample_size = size;
    sample = data;
    type = MediaAudio;
}

void AVMediaData::setVideo(uint8_t *data, int length, int width, int height, int pixelFormat) {
    image = data;
    this->length = length;
    this->width = width;
    this->height = height;
    this->pixelFormat = pixelFormat;
    type = MediaVideo;
}

const char* AVMediaData::getName() {
    return get_media_type_string(getType());
}

MediaType AVMediaData::getType() {
    return type;
}

int64_t AVMediaData::getPts() {
    return pts;
}
