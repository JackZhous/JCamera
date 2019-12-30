//
// Created by jackzhous on 2019/12/27.
//

#ifndef JCAMERA_YUVCONVERTOR_H
#define JCAMERA_YUVCONVERTOR_H

#include "YUVData.h"
#include "AVFormatter.h


class YUVConvertor {
public:
    YUVConvertor();

    ~YUVConvertor();

    void reset();

    void release();

    void setInputParams(int width, int height, int pixelFormat);

    void setCrop(int x, int y, int width, int height);

    void setRotate(int degree);

    void setScale(int width, int height);

    void setMirror(bool mirror);

    int prepare();

    int getOutputWidth();

    int getOutputHeight();


private:
    int mWidth;
    int mHeight;
    int mPixFormat;
    bool mNeedConvert;      //是否需要转换

    //裁剪
    int mCropX;
    int mCropY;
    int mCropWidth;
    int mCropHeight;

    libyuv::RotationMode mRotationMode;
    int mScaleWidth;
    int mScaleHeight;
    bool mMirror;

    YUVData *pCropData;
    YUVData *pScaleData;
    YUVData *pMirrorata;
};


#endif //JCAMERA_YUVCONVERTOR_H
