//
// Created by jackzhous on 2019-09-11.
//

#ifndef MYPLAYER_EGLHELPER_H
#define MYPLAYER_EGLHELPER_H

/**
 * Constructor flag: surface must be recordable.  This discourages EGL from using a
 * pixel format that cannot be converted efficiently to something usable by the video
 * encoder.
 */
#define FLAG_RECORDABLE 0x01    //可录制

/**
 * Constructor flag: ask for GLES3, fall back to GLES2 if not available.  Without this
 * flag, GLES2 is used.
 */
#define FLAG_TRY_GLES3 002

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#include "MediaEGLContext.h"

typedef EGLBoolean (EGLAPIENTRYP EGL_PRESENTATION_TIME_ANDROIDPROC)(EGLDisplay display, EGLSurface surface, khronos_stime_nanoseconds_t time);

class EGLHelper{
    public:
        EGLHelper();
        ~EGLHelper();

        bool init(int flags);

        bool init(EGLContext shareContext, int flags);
        //释放资源
        void release();

        EGLContext  getEglContext();

        void destroySurface(EGLSurface eglSurface);

        // 切换到某个上下文
        void makeCurrent(EGLSurface drawSurface, EGLSurface readSurface);

        //销毁surface
        EGLSurface createSurface(ANativeWindow* surface);

        //创建离屏surface
        EGLSurface createSurface(int width, int height);

        //切换到上下文
        void makeCurrent(EGLSurface eglSurface);

        //没有上下文
        void makeNothingCurrent();

        int swapBuffers(EGLSurface eglSurface);
        //设置pts
        void setPresentationTime(EGLSurface eglSurface, long nses);

        //判断是否属于当前上下文
        bool isCurrent(EGLSurface surface);

        //查询字符串
        const char *queryString(int what);
        //执行查询
        int querySurface(EGLSurface eglSurface, int what);

        int getEGLVersion();

        void checkEglError(const char* msg);

    private:
        //查找合适的EGLConfig
        EGLConfig  getConfig(int flags, int version);


    private:
        EGLDisplay mEglDisplay;
        EGLConfig mEglConfig;
        EGLContext mEglContext;
        int mEglVersion;

        EGL_PRESENTATION_TIME_ANDROIDPROC eglPresentationTimeANDROID;
};


#endif //MYPLAYER_EGLHELPER_H