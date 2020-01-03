//
// Created by jackzhous on 2019/12/27.
//

#include <AndroidLog.h>
#include <opencl-c.h>
#include "YUVConvertor.h"


YUVConvertor::YUVConvertor() {
    reset();
}


YUVConvertor::~YUVConvertor() {
    release();
}

void YUVConvertor::reset() {
    mNeedConvert = false;
    mCropHeight = 0;
    mCropWidth = 0;
    mCropX = 0;
    mCropY = 0;
    mRotationMode = libyuv::kRotate0;
    mScaleHeight = 0;
    mScaleWidth = 0;
    mMirror = false;

    pCropData = nullptr;
    pMirrorata = nullptr;
    pScaleData = nullptr;
}


void YUVConvertor::release() {
    if(pCropData != nullptr){
        delete pCropData;
        pCropData = nullptr;
    }

    if(pScaleData != nullptr){
        delete pScaleData;
        pScaleData = nullptr;
    }

    if(pMirrorata != nullptr){
        delete pMirrorata;
        pMirrorata = nullptr;
    }
    reset();
}

void YUVConvertor::setInputParams(int width, int height, int pixelFormat) {
    mWidth = width;
    mHeight = height;
    mPixFormat = pixelFormat;
}

void YUVConvertor::setCrop(int x, int y, int width, int height) {
    mCropY = y;
    mCropX = x;
    mCropWidth = width;
    mCropHeight = height;
}

void YUVConvertor::setRotate(int degree) {
    mRotationMode = getRotationMode(degree);
}

void YUVConvertor::setScale(int width, int height) {
    mScaleWidth = width;
    mScaleHeight = height;
}

void YUVConvertor::setMirror(bool mirror) {
    mMirror = mirror;
}


int YUVConvertor::convert(AVMediaData *mediaData) {
    if(!mNeedConvert){
        LOGE("unable to convert media data");
        return -1;
    }

    if(mediaData->getType() != MediaVideo){
        LOGE("failed to convert media data: %s", mediaData->getName());
        return -1;
    }

    if(mCropX + mCropWidth > mediaData->width || mCropY + mCropHeight > mediaData->height){
        LOGE("crop argument invalid. media data: [%d ,%d], crop: [%d, %d ,%d ,%d]",
             mediaData->width, mediaData->height,
             mCropX, mCropY, mCropWidth, mCropHeight);
        return -1;
    }

    int ret = 0;
    ret = ConvertToI420(mediaData->image, (size_t) mediaData->length,
            pCropData->dataY, pCropData->lineSizeY,
            pCropData->dataU, pCropData->lineSizeU,
            pCropData->dataV, pCropData->lineSizeV,
            mCropX, mCropY, mediaData->width, mediaData->height, mCropWidth, mCropHeight,
            mRotationMode, getFourCC((PixelFormat)mediaData->pixelFormat));
    if(ret < 0){
        LOGE("failed to call convertT420: %d", ret);
        return ret;
    }
    YUVData *src = pCropData;
    YUVData *output = pCropData;
    int outputWidth = (mRotationMode == libyuv::kRotate0 || mRotationMode == libyuv::kRotate180) ? mCropWidth : mCropHeight;
    int outputHeight = (mRotationMode == libyuv::kRotate0 || mRotationMode == libyuv::kRotate180) ? mCropHeight : mCropWidth;

    if(mScaleWidth > 0 && mScaleHeight > 0){
        if(scale(src, outputWidth, outputHeight) < 0){
            return -1;
        }
        src = pScaleData;
        output = pScaleData;
        outputWidth = mScaleWidth;
        outputHeight = mScaleHeight;
    }

    if(mMirror){
        if(mirror(src, outputWidth, outputHeight) < 0){
            return -1;
        }
        output = pMirrorata;
    }

    fillMediaData(mediaData, output, outputWidth, outputHeight);
}


/**
 * 填充媒体数据
 * @param model
 * @param src
 * @param srcW
 * @param srcH
 */
void YUVConvertor::fillMediaData(AVMediaData *model, YUVData *src, int srcW, int srcH) {
    uint8_t *image = new uint8_t[srcW * srcH *3 / 2];
    if(model != nullptr){
        model->free();
    } else{
        model = new AVMediaData();
    }
    model->image = image;
    memcpy(model->image, src->dataY, (size_t)srcW * srcH);
    memcpy(model->image+ srcW* srcH, src->dataU, (size_t)srcW * srcH / 4);
    memcpy(model->image+ srcW* srcH * 5 /4, src->dataV, (size_t)srcW * srcH / 4);
    model->length = srcW * srcH * 3 / 2;
    model->width = srcW;
    model->height = srcH;
    model->pixelFormat = PIXEL_FORMAT_YUV420P;
}



int YUVConvertor::mirror(YUVData *src, int srcW, int srcH) {
    int ret;
    ret = libyuv::I420Mirror(src->dataY, src->lineSizeY,
                             src->dataU, src->lineSizeU,
                             src->dataV, src->lineSizeV,
                             pMirrorata->dataY, pMirrorata->lineSizeY,
                             pMirrorata->dataU, pMirrorata->lineSizeU,
                             pMirrorata->dataV, pMirrorata->lineSizeV,
                             srcW, srcH);
    if (ret < 0) {
        LOGE("Failed to call I420Mirror: %d", ret);
        return ret;
    }
    return 0;
}

/**
 * 缩放处理
 * @param src
 * @param srcW
 * @param srcH
 * @return
 */
int YUVConvertor::scale(YUVData *src, int srcW, int srcH) {
    int ret;
    ret = libyuv::I420Scale(src->dataY, src->lineSizeY,
                            src->dataU, src->lineSizeU,
                            src->dataV, src->lineSizeV,
                            srcW, srcH,
                            pScaleData->dataY, pScaleData->lineSizeY,
                            pScaleData->dataU, pScaleData->lineSizeU,
                            pScaleData->dataV, pScaleData->lineSizeV,
                            mScaleWidth, mScaleHeight,
                            libyuv::kFilterBox);
    if (ret < 0) {
        LOGE("Failed to call I420Scale: %d", ret);
        return ret;
    }

    return 0;
}

/**
 * 准备转换器
 */
int YUVConvertor::prepare() {
    if((mCropWidth == 0 || mCropHeight == 0)
            && (mRotationMode == libyuv::kRotate0)
            && (mScaleHeight ==0 && mScaleWidth == 0)
            && !mMirror
            && mPixFormat == PIXEL_FORMAT_YUV420P){
        mNeedConvert = false;
        return -1;
    }
    mNeedConvert = true;

    //裁剪宽高不存在，用源宽高处理
    if(mCropHeight == 0 && mCropWidth == 0){
        mCropHeight = mHeight;
        mCropWidth = mWidth;
    }

    //限定裁剪高度为偶数
    if(mCropWidth % 2 == 1){
        if(mCropHeight >= mCropWidth){
            //等比例mCropWidth变为偶数收求取mCropHeight
            mCropHeight = (int)(1.0 * (mCropWidth - 1) / mCropWidth * mCropHeight);
            mCropHeight = mCropHeight % 2 == 1 ? mCropHeight - 1 : mCropWidth;
        }
        mCropWidth--;
    }

    if(mCropHeight % 2 == 1){
        if(mCropWidth >= mCropHeight){
            mCropWidth = (int)(1.0 * (mCropHeight - 1)/ mCropHeight * mCropWidth);
            mCropHeight = mCropHeight % 2 == 1 ? mCropWidth - 1 : mCropWidth;
        }
        mCropHeight--;
    }

    //创建yuv裁剪对象
    if(mCropWidth > 0 && mCropHeight >0){
        //竖屏
        int width = (mRotationMode == libyuv::kRotate0 || mRotationMode == libyuv::kRotate180)
                        ? mCropWidth : mCropHeight;
        int height = (mRotationMode == libyuv::kRotate0 || mRotationMode == libyuv::kRotate180)
                    ? mCropHeight : mCropWidth;
        pCropData = new YUVData();
        pCropData->alloc(width, height);
    }

    if(mScaleWidth>0 && mScaleHeight > 0){
        pScaleData = new YUVData();
        pScaleData->alloc(mScaleWidth, mScaleHeight);
    }

    if(mMirror){
        pMirrorata = new YUVData();
        pMirrorata->alloc(getOutputWidth(), getOutputHeight());
    }

    return 0;
}


 int YUVConvertor::getOutputWidth() {
    int width = mScaleWidth;
    if(width == 0){
        width = (mRotationMode == libyuv::kRotate0 || mRotationMode == libyuv::kRotate180)
                        ? mCropWidth : mCropHeight;
    }
    return width;
}

int YUVConvertor::getOutputHeight() {
    int height = mScaleWidth;
    if(height == 0){
        height = (mRotationMode == libyuv::kRotate0 || mRotationMode == libyuv::kRotate180)
                ? mCropHeight : mCropWidth;
    }
    return height;
}