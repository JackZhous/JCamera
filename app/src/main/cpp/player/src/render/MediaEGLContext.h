//
// Created by jackzhous on 2019-09-11.
//

#ifndef MYPLAYER_MEDIAEGLCONTEXT_H
#define MYPLAYER_MEDIAEGLCONTEXT_H

#include <android/native_window.h>
#include <EGL/eglplatform.h>
#include <EGL/eglext.h>
#include <EGL/egl.h>
#include "EGLHelper.h"
#include <mutex>
#include <AndroidLog.h>


/**
 * EGLContext 上下文，为了方便使用SharedContext而造的
 */
class MediaEGLContext{
    public:
        static MediaEGLContext *getInstance();

        void destroy();

        EGLContext  getContext();



private:
        MediaEGLContext();

        virtual ~MediaEGLContext();

        bool init(int flags);

        void release();

        EGLConfig getConfig(int flags, int version);

        void checkEglError(const char *msg);
        static MediaEGLContext *instance;
        static std::mutex mutex;

        EGLContext eglContext;
        EGLDisplay  eglDisplay;
};

#endif //MYPLAYER_MEDIAEGLCONTEXT_H
