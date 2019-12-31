//
// Created by jackzhous on 2019/12/27.
//

#ifndef JCAMERA_AVMEDIAWRITER_H
#define JCAMERA_AVMEDIAWRITER_H


#include <map>
#include <libavutil/pixfmt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

class AVMediaWriter {
public:
    AVMediaWriter();

    virtual ~AVMediaWriter();

    void release();

    void setOutoutPath(const char* dstUrl);

    void setMUseTimeStamp(bool mUseTimeStamp);

    void setMMaxBitRate(int64_t mMaxBitRate);

    /**
     * 添加编码参数
     */
    void addEncodeOptions(std::string key, std::string value);

    //指定音频编码器
    void setAudioEncoderName(const char *encoder);
    void setVideoEncodeName(const char *coder);
    void setQuality(int quality);

    void setOutputVideo(int width, int height, int frameRate, AVPixelFormat pixFormat);

    void setOutputAudio(int sampleRate, int channel, AVSampleFormat sampleFormat);

    int prepare();


private:
    void reset();

    int openOutputFile();

    int openEncoder(AVMediaType type);

private:
    std::map<std::string, std::string> mEncodeOptions;          //编码参数
    std::map<std::string, std::string> mVideoMetadata;          //视频meta数据

    const char* mDstUrl;

    int mWidth;
    int mHeight;
    int mFrameRate;
    AVPixelFormat  mPixelFormat;
    int64_t mMaxBitRate;
    AVCodecID mVideoCodecId;
    const char *mVideoEncodeName;                   //指定编码器名称
    bool mUseTimeStamp;                     //是否使用时间戳计算pts
    bool mHasVideo;                         //是否存在视频流数据

    int mSampleRate;
    int mChannels;
    AVSampleFormat mSampleFormat;
    const char *mAudioEncodeName;
    AVCodecID mAudioCodecId;
    bool mHasAudio;

    //编码上下文
    AVFormatContext *pFormatCtx;
    AVCodecContext *pVideoCodecCtx;
    AVCodecContext *pAudioCodecCtx;
    AVStream *pVideoStream;                 //视频流
    AVStream *pAudioStream;                 //音频流

    SwrContext *pSampleConvertCtx;          //转码上下文
    AVFrame *mSampleFrame;                  //音频缓冲帧
    uint8_t **mSampleBuffer;                //音频缓冲区
    int mSampleSize;                        //采样大小
    int mSamplePlanes;                      //每个采样点数量
    int mNbSamples;                         //采样数量

    AVFrame *mImageFrame;                   //视频缓冲帧
    uint8_t *mImageBuffer;                  //视频缓冲区
    int mImageCount;                        //视频数量
    int64_t mStartPts;                      //开始的Pts
    int64_t mLastPts;                       //上一帧
};


#endif //JCAMERA_AVMEDIAWRITER_H
