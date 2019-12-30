//
// Created by jackzhous on 2019/12/30.
//

#ifndef JCAMERA_AVFORMATTER_H
#define JCAMERA_AVFORMATTER_H

#include <libyuv/rotate.h>
extern "C"{
    #include <libavutil/samplefmt.h>
};

/**
 * YUV planner平面格式俗称YUVP YU12 YV12
 * YU12 yyyyuuvv   YV12 yyyyvvuu
 * YUV package混合模式俗称YUVSP  NV12 NV21
 * NV12 yyyyuvuv NV21 yyyyvuvu
 */
enum PixelFormat {
    PIXEL_FORMAT_NONE    = 0,
    PIXEL_FORMAT_NV21    = 1,
    PIXEL_FORMAT_YV12    = 2,
    PIXEL_FORMAT_NV12    = 3,
    PIXEL_FORMAT_YUV420P = 4,
    PIXEL_FORMAT_ARGB    = 5,
    PIXEL_FORMAT_ABGR    = 6,
    PIXEL_FORMAT_RGBA    = 7,
};


enum SampleFormat{
    SAMPLE_FORMAT_8BIT = 8,
    SAMPLE_FORMAT_16BIT = 16,
    SAMPLE_FORMAT_FLOAT = 32,
};

/**
 * 获取视频像素格式
 */
inline AVPixelFormat getPixelFormat(PixelFormat format){
    AVPixelFormat pixelFormat = AV_PIX_FMT_NONE;
    switch (format){
        case PIXEL_FORMAT_NV21:
            pixelFormat = AV_PIX_FMT_NV12;
            break;

        case PIXEL_FORMAT_NV12:
            pixelFormat = AV_PIX_FMT_NV12;
            break;

        case PIXEL_FORMAT_YUV420P:
            pixelFormat = AV_PIX_FMT_YUV420P;
            break;
    }

    return pixelFormat;
}

inline AVSampleFormat getSampleFormate(SampleFormat format){
    AVSampleFormat sampleFormat = AV_SAMPLE_FMT_NONE;
    switch (format){
        case SAMPLE_FORMAT_8BIT:
            sampleFormat = AV_SAMPLE_FMT_U8;
            break;

        case SAMPLE_FORMAT_16BIT:
            sampleFormat = AV_SAMPLE_FMT_S16;
            break;

        case SAMPLE_FORMAT_FLOAT:
            sampleFormat = AV_SAMPLE_FMT_FLT;
            break;
    }
    return sampleFormat;
}


inline libyuv::RotationMode getRotationMode(int degree){
    switch (degree){
        case 90:
            return libyuv::kRotate90;

        case 180:
            return libyuv::kRotate180;

        case 270:
            return libyuv::kRotate270;

        default:
            return libyuv::kRotate0;
    }
}


#endif //JCAMERA_AVFORMATTER_H
