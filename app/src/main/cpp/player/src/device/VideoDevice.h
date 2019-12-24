//
// Created by jackzhous on 2019/8/27.
//




/**
 * GLSL opengl纹理显示
 */
#ifndef MYPLAYER_VIDEODEVICE_H
#define MYPLAYER_VIDEODEVICE_H
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "render/EGLHelper.h"

#include <Thread.h>
#include <render/GLInputFilter.h>
#include <render/InputRenderNode.h>

class VideoDevice{
    public:
        VideoDevice();
        ~VideoDevice();

        void surfaceCreate(ANativeWindow* window);

        void terminate(bool flag);

        void onInitTexture(int width, int height, TextureFormat format, BlendMode blendMode, int rotate = 0);

        int onUpdateYUV(uint8_t *yData, int yPitch, uint8_t *uData, int uPitch, uint8_t *vData, int vPitch);

        int onUpdateARGB(uint8_t *rgba, int pitch);

        int onRequestRender(bool flip);

    private:
        void resetVertices();

        void resetTextureVertices();

    private:
        Mutex mMutex;
        Condition mCond;

        ANativeWindow *mWindow;
        int mSurfaceWidth;
        int mSurfaceHeight;
        EGLSurface eglSurface;
        EGLHelper* eglHelper;
        bool mSurfaceReset;                 //重新设置surface标志
        bool mHasSurface;                   //是否存在Surface， java层传递过来的surface
        bool mHasEGLSurface;               //是否存在EGL Surface
        bool mHasEGLContext;

        Texture* mVideoTexture;             //视频纹理
        InputRenderNode *mRenderNode;       //渲染视频节点
        float vertives[8];                  //顶点坐标
        float textureVertices[8];           //纹理坐标
};

#endif //MYPLAYER_VIDEODEVICE_H
