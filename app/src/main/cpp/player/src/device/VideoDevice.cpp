//
// Created by jackzhous on 2019/8/27.
//

#include "VideoDevice.h"


VideoDevice::VideoDevice() {
    mWindow = NULL;
    mSurfaceHeight = 0;
    mSurfaceWidth = 0;
    eglSurface = EGL_NO_SURFACE;
    eglHelper = new EGLHelper();
    mHasEGLContext = false;
    mHasEGLSurface = false;
    mHasSurface = false;

    mVideoTexture = (Texture*)malloc(sizeof(Texture));
    memset(mVideoTexture, 0, sizeof(Texture));
    mRenderNode = NULL;

    resetVertices();
    resetTextureVertices();
}


VideoDevice::~VideoDevice() {
    mMutex.lock();
    terminate(true);
    mMutex.unlock();
}


void VideoDevice::terminate(bool release) {
    if(eglSurface != EGL_NO_SURFACE){
        eglHelper->destroySurface(eglSurface);
        eglSurface = EGL_NO_SURFACE;
        mHasSurface = false;
    }

    if(eglHelper->getEglContext() != EGL_NO_CONTEXT && release){
        if(mRenderNode){
            mRenderNode->destroy();
            delete mRenderNode;
        }
        eglHelper->release();
        mHasEGLContext = false;
    }
}

void VideoDevice::resetVertices() {
    const float *ver = CoordinateUtils::getVertexCoordinates();
    for (int i = 0; i < 8; ++i) {
        vertives[i] = ver[i];
    }
}

void VideoDevice::resetTextureVertices() {
    const float *ver = CoordinateUtils::getTextureCoordinates(RORATE_NONE);
    for (int i = 0; i < 8; ++i) {
        textureVertices[i] = ver[i];
    }
}

/**
 * 初始化纹理以及renderNode内部初始化
 * @param width
 * @param height
 * @param format
 * @param blendMode
 * @param rotate
 */
void VideoDevice::onInitTexture(int width, int height, TextureFormat format, BlendMode blendMode,
                                int rotate) {
    mMutex.lock();

    //创建EGLContext
    if(!mHasEGLContext){
        mHasEGLContext = eglHelper->init(FLAG_TRY_GLES3);
        LOGI("mHasEGLContext = %d", mHasEGLContext);
    }

    if(!mHasEGLContext){
        mMutex.unlock();
        return;
    }

    //重新设置Surface
    if(mHasSurface && mSurfaceReset){
        terminate(false);
        mSurfaceReset = false;
    }

    //创建/释放EGLSurface
    if(eglSurface == EGL_NO_SURFACE && mWindow != NULL){
        if(mHasSurface && !mHasEGLSurface){
            eglSurface = eglHelper->createSurface(mWindow);
            if(eglSurface != EGL_NO_SURFACE){
                mHasEGLSurface = true;
                LOGI("mHasEGLSurface = %d", mHasEGLSurface);
            }
        }
    } else if(eglSurface != EGL_NO_SURFACE && mHasEGLSurface){
        //处于destroy状态，释放eglsurface
        if(!mHasSurface){
            terminate(false);
        }
    }

    //计算帧宽高，如果不相等，则需要重新计算缓冲区大小
    if(mWindow != NULL && mSurfaceWidth != 0 && mSurfaceHeight != 0){
        //宽高比不一致，需要调整缓冲区大小，以宽度为基准
        if((mSurfaceWidth / mSurfaceHeight) != (width/height)){
            LOGI("width height %d -- %d -- %d --- %d", mSurfaceWidth, mSurfaceHeight, width, height);
            mSurfaceHeight = mSurfaceWidth * height / width;
            LOGI("width height %d -- %d -- %d --- %d", mSurfaceWidth, mSurfaceHeight, width, height);
            int windowFormat = ANativeWindow_getFormat(mWindow);
            ANativeWindow_setBuffersGeometry(mWindow, mSurfaceWidth, mSurfaceHeight, windowFormat);
        }
    }

    mVideoTexture->rotate = rotate;
    mVideoTexture->frameWidth = width;
    mVideoTexture->frameHeight = height;
    mVideoTexture->height = height;
    mVideoTexture->format = format;
    mVideoTexture->blendMode = blendMode;
    mVideoTexture->direction = FLIP_NONE;
    eglHelper->makeCurrent(eglSurface);
    if(mRenderNode == NULL){
        mRenderNode = new InputRenderNode();
        if(mRenderNode != NULL){
            mRenderNode->initFilter(mVideoTexture);
        }
    }

    mMutex.unlock();
}


int VideoDevice::onUpdateYUV(uint8_t *yData, int yPitch, uint8_t *uData, int uPitch, uint8_t *vData,
                             int vPitch) {
    if(!mHasEGLContext){
        return -1;
    }

    mMutex.lock();
    mVideoTexture->pitches[0] = yPitch;
    mVideoTexture->pitches[1] = uPitch;
    mVideoTexture->pitches[2] = vPitch;
    mVideoTexture->pixels[0] = yData;
    mVideoTexture->pixels[1] = uData;
    mVideoTexture->pixels[2] = vData;
    if(mRenderNode != NULL && eglSurface != EGL_NO_SURFACE){
        eglHelper->makeCurrent(eglSurface);
        mRenderNode->uploadTexture(mVideoTexture);
    }
    mVideoTexture->width = yPitch;
    mMutex.unlock();
    return 0;
}


int VideoDevice::onUpdateARGB(uint8_t *rgba, int pitch) {
    if (!mHasEGLContext) {
        return -1;
    }
    mMutex.lock();
    mVideoTexture->pitches[0] = pitch;
    mVideoTexture->pixels[0] = rgba;
    if (mRenderNode != NULL && eglSurface != EGL_NO_SURFACE) {
        eglHelper->makeCurrent(eglSurface);
        mRenderNode->uploadTexture(mVideoTexture);
    }
    // 设置像素实际的宽度，即linesize的值
    mVideoTexture->width = pitch / 4;
    mMutex.unlock();
    return 0;
}

int VideoDevice::onRequestRender(bool flip) {
    if (!mHasEGLContext) {
        return -1;
    }
    mMutex.lock();
    mVideoTexture->direction = flip ? FLIP_VERTICAL : FLIP_NONE;
    if (mRenderNode != NULL && eglSurface != EGL_NO_SURFACE) {
        eglHelper->makeCurrent(eglSurface);
        if (mSurfaceWidth != 0 && mSurfaceHeight != 0) {
            mRenderNode->setDisplaySize(mSurfaceWidth, mSurfaceHeight);
        }
        mRenderNode->drawFrame(mVideoTexture);
        eglHelper->swapBuffers(eglSurface);
//        LOGI("swap 显示图像");
    }
    mMutex.unlock();
    return 0;
}


void VideoDevice::surfaceCreate(ANativeWindow *window) {
    mMutex.lock();
    if(mWindow != NULL){
        ANativeWindow_release(mWindow);
        mWindow = NULL;
        mSurfaceReset = true;
    }

    mWindow = window;
    if(mWindow != NULL){
        mSurfaceWidth = ANativeWindow_getWidth(window);
        mSurfaceHeight = ANativeWindow_getHeight(window);
    }
    mHasSurface = true;
    LOGI("set surface ");
    mMutex.unlock();
}