//
// Created by jackzhous on 2019/12/27.
//

#include <sstream>
#include <AndroidLog.h>
#include "AVMediaWriter.h"

AVMediaWriter::~AVMediaWriter() {
    release();
}

AVMediaWriter::AVMediaWriter() {
    reset();
}

void AVMediaWriter::setOutoutPath(const char *dstUrl) {
    mDstUrl = dstUrl;
}

void AVMediaWriter::addEncodeOptions(std::string key, std::string value) {
    mEncodeOptions[key] = value;
}

void AVMediaWriter::setAudioEncoderName(const char *encoder) {
    mAudioEncodeName = av_strdup(encoder);
}

void AVMediaWriter::setVideoEncodeName(const char *coder) {
    mVideoEncodeName = av_strdup(coder);
}

void AVMediaWriter::setQuality(int quality) {
    //字符串转换流
    std::stringstream ss;
    ss << quality;
    std::string str;
    ss >> str;
    mEncodeOptions["crf"] = str;
}


void AVMediaWriter::setOutputVideo(int width, int height, int frameRate, AVPixelFormat pixFormat) {
    mWidth = width;
    mHeight = height;
    mPixelFormat = pixFormat;
    mFrameRate = frameRate;
    mVideoCodecId = AV_CODEC_ID_H264;
    mHasVideo = mWidth > 0 && mHeight > 0 && mPixelFormat != AV_PIX_FMT_NONE;
}

void AVMediaWriter::setOutputAudio(int sampleRate, int channel, AVSampleFormat sampleFormat) {
    mSampleRate = sampleRate;
    mChannels = channel;
    mSampleFormat = sampleFormat;
    mAudioCodecId = AV_CODEC_ID_AAC;
    mHasAudio = mSampleRate > 0 || mChannels > 0 && mSampleFormat != AV_SAMPLE_FMT_NONE;
}


int AVMediaWriter::prepare() {
    //去掉奇数宽度，为什么必须是偶数 因为YUV420的高宽必须为偶数，两行公用UV 一行取U 一行取V
    if(mWidth % 2 == 1){
        if(mHeight >= mWidth){
            mHeight = (int) (1.0 * (mWidth - 1) / mWidth * mHeight);
            mHeight = mHeight % 2 == 1 ? mHeight - 1 : mHeight;
        }
        mWidth--;
    }

    if(mHeight % 2 == 1){
        if(mWidth >= mHeight){
            mWidth = (int) (1.0 * (mHeight - 1) / mWidth * mHeight);
            mWidth = mWidth % 2 == 1 ? mWidth - 1 : mWidth;
        }
        mHeight--;
    }
    //打开输出文件
    return openOutputFile();
}

int AVMediaWriter::openOutputFile() {
    int ret;
    av_register_all();

    ret = avformat_alloc_output_context2(&pFormatCtx, nullptr, nullptr, mDstUrl);
    if(ret < 0 || !pFormatCtx){
        LOGE("failed to call avformat_alloc_output_context2: %s", av_err2str(ret));
        return AVERROR_UNKNOWN;
    }

    //打开视频编码器
    if(mHasVideo && (ret = openEncoder(AVMEDIA_TYPE_VIDEO) < 0)){
        LOGE("failed to Open video encoder context: %s", av_err2str(ret));
        return ret;
    }

    return ret;
}

int AVMediaWriter::openEncoder(AVMediaType type) {

}


void AVMediaWriter::reset() {
    mDstUrl = nullptr;
    mWidth = 0;
    mHeight = 0;
    mFrameRate = 0;
    mPixelFormat = AV_PIX_FMT_NONE;
    mVideoCodecId = AV_CODEC_ID_NONE;
    mVideoEncodeName = nullptr;
    mUseTimeStamp = false;
    mHasVideo = false;

    mSampleRate = 0;
    mChannels = 0;
    mSampleFormat = AV_SAMPLE_FMT_NONE;
    mAudioEncodeName = nullptr;
    mAudioCodecId = AV_CODEC_ID_NONE;
    mHasAudio = false;

    pFormatCtx = nullptr;
    pVideoCodecCtx = nullptr;
    pAudioCodecCtx = nullptr;
    pVideoStream = nullptr;
    pAudioStream = nullptr;

    mNbSamples = 0;
    mSampleFrame = nullptr;
    mSampleBuffer = nullptr;
    mSampleSize = 0;
    mSamplePlanes = 0;
    pSampleConvertCtx = nullptr;

    mImageFrame = nullptr;
    mImageBuffer = nullptr;
    mImageCount = 0;
    mStartPts = 0;
    mLastPts = -1;
}

void AVMediaWriter::release() {
    if(mImageFrame != nullptr){
        av_frame_free(&mImageFrame);
        mImageFrame = nullptr;
    }

    if(mImageBuffer != nullptr){
        av_free(mImageBuffer);
        mImageBuffer = nullptr;
    }

    if(mSampleFrame != nullptr){
        av_frame_free(&mSampleFrame);
        mSampleFrame = nullptr;
    }

    if(mSampleBuffer != nullptr){
        for(int i = 0; i < mSamplePlanes; i++){
            av_free(mSampleBuffer[i]);
            mSampleBuffer[i] = nullptr;
        }
        delete[] mSampleBuffer;
        mSampleBuffer = nullptr;
    }

    if(pVideoCodecCtx != nullptr){
        avcodec_free_context(&pVideoCodecCtx);
        pVideoCodecCtx = nullptr;
    }

    if(pAudioCodecCtx != nullptr){
        avcodec_free_context(&pAudioCodecCtx);
        pAudioCodecCtx = nullptr;
    }

    if(pFormatCtx && !(pFormatCtx->oformat->flags & AVFMT_NOFILE)){
        avio_closep(&pFormatCtx->pb);
        avformat_close_input(&pFormatCtx);
        pFormatCtx = nullptr;
    }

    if(pSampleConvertCtx != nullptr){
        swr_free(&pSampleConvertCtx);
        pSampleConvertCtx = nullptr;
    }

    if(pVideoStream != nullptr && pVideoStream->metadata != nullptr){
        av_dict_free(&pVideoStream->metadata);
        pVideoStream->metadata = nullptr;
    }
    pVideoStream = nullptr;
    pAudioStream = nullptr;
}

void AVMediaWriter::setMUseTimeStamp(bool mUseTimeStamp) {
    AVMediaWriter::mUseTimeStamp = mUseTimeStamp;
}

void AVMediaWriter::setMMaxBitRate(int64_t mMaxBitRate) {
    AVMediaWriter::mMaxBitRate = mMaxBitRate;
}
