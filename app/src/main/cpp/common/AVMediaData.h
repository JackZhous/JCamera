//
// Created by jackzhous on 2019/12/27.
//

#ifndef JCAMERA_AVMEDIADATA_H
#define JCAMERA_AVMEDIADATA_H


#include <cstdint>
#include "AVFormatter.h"

enum MediaType {
    MediaNone = -1,
    MediaAudio = 0,
    MediaVideo = 1
};


inline const char* get_media_type_string(MediaType type){
    if(type == MediaNone){
        return "MediaNone";
    } else if (type == MediaAudio){
        return "MediaAudio";
    } else if (type == MediaVideo){
        return "MediaVideo";
    } else{
        return "Unknown";
    }
}

class AVMediaData {
public:
    AVMediaData();

    virtual ~AVMediaData();

    void setVideo(uint8_t *data, int length, int width, int height, int pixelFormat);

    void setAudio(uint8_t *data, int size);

    void setPts(int64_t pts);

    int64_t getPts();

    MediaType getType();

    const char *getName();

    void free();

private:
    uint8_t  *image;
    int length;

    uint8_t  *sample;
    int sample_size;

    int width;
    int height;
    int pixelFormat;
    int64_t pts;
    MediaType type;
};


#endif //JCAMERA_AVMEDIADATA_H
