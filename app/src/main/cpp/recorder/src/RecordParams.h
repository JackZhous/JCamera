//
// Created by jackzhous on 2019/12/27.
//

#ifndef JCAMERA_RECORDPARAMS_H
#define JCAMERA_RECORDPARAMS_H


#include <cstdint>

extern "C"{
    #include <libavutil/mem.h>
};

class RecordParams {
public:
    const char* dstFile;            //输出路径
    int width;
    int height;
    int frameRate;                  //帧率
    int pixFormat;                  //像素格式
    int64_t maxRate;
    int quality;                    //质量系数
    bool enableVideo;               //是否允许视频编码
    const char* videoEncoder;       //视频编码器名称

    //音频参数
    int sampleRate;                 //采样率
    int channels;
    int sampleFormat;
    bool enableAudio;
    const char* audioEncoder;

    int cropX;
    int cropY;                      //裁剪XY 坐标
    int cropWidth;
    int cropHeight;
    int rotateDegree;
    int scaleWidth;
    int scaleHeight;
    bool mirror;                    //是否需要镜像处理

    const char* videoFilter;
    const char* audioFilter;            //滤镜描述
    bool isFrontCamera;                 //是否为前置社小偷


    public:
        RecordParams();

        virtual ~RecordParams();

        void setOutput(const char *url);

        //设置视频参数
        void setVideoParams(int width, int height, int frameRate, int pixFormat, int maxBitRate, int quality);

        //设置音频参数
        void setAudioParams(int sampleRate, int sampleFormat, int channel);

        void setVideoEncoder(const char* coder);

        void setAudioEncoder(const char* coder);

        void setMaxBitRate(int bitRate);

        void setQuality(int quality);

        void setCrop(int x, int y, int cropW, int cropH);

        void setRotate(int degree);

        void setScale(int scaleW, int scaleH);

        void setMirror(bool mirror);

        void setVideoFilter(const char* filter);

        void setAudioFilter(const char* filter);


};


#endif //JCAMERA_RECORDPARAMS_H
