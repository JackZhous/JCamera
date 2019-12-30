//
// Created by jackzhous on 2019/12/27.
//

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