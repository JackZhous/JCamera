//
// Created by jackzhous on 2019-09-11.
//

#include "MediaEGLContext.h"

MediaEGLContext* MediaEGLContext::instance = NULL;
std::mutex MediaEGLContext::mutex;

MediaEGLContext::MediaEGLContext() {
    eglDisplay = EGL_NO_DISPLAY;
    eglContext = EGL_NO_CONTEXT;
    init(FLAG_TRY_GLES3);
}


MediaEGLContext::~MediaEGLContext() {}

MediaEGLContext* MediaEGLContext::getInstance() {
    if(!instance){
        std::unique_lock<std::mutex> locks(mutex);
        if(!instance){
            instance = new  MediaEGLContext();
        }
    }

    return instance;
}

bool MediaEGLContext::init(int flags) {
    if(eglDisplay != EGL_NO_DISPLAY){
        LOGE("eglDisplay is already set up");
        return false;
    }

    //获取display
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(eglDisplay == EGL_NO_DISPLAY){
        LOGE("unable to create EGLDisplay");
        return false;
    }

    //初始化EGLDisplay
    if(!eglInitialize(eglDisplay, 0, 0)){
        eglDisplay = EGL_NO_DISPLAY;
        LOGE("unable to initialize EGLDisplay");
        return false;
    }

    //判断是否尝试使用GLES3
    if((flags & FLAG_TRY_GLES3) != 0){
        EGLConfig config = getConfig(flags, 3);
        if(config != NULL){
            int attribute3_list[] = {
                    EGL_CONTEXT_CLIENT_VERSION, 3,
                    EGL_NONE
            };
            EGLContext context = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, attribute3_list);
            checkEglError("eglCreateContext");
            if(eglGetError() == EGL_SUCCESS){
                eglContext = context;
            }
        }
    }

    if(eglContext == EGL_NO_CONTEXT){
        EGLConfig config = getConfig(flags, 2);
        int attribute2_list[] = {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_NONE
        };
        EGLContext context = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, attribute2_list);
        checkEglError("eglCreateContext");
        if(eglGetError() == EGL_SUCCESS){
            eglContext = context;
        }
    }

    int value[1] = {0};
    eglQueryContext(eglDisplay, eglContext, EGL_CONTEXT_CLIENT_VERSION, value);
    LOGI("EGLContext created, the client version is %d", value[0]);
    return true;
}


EGLConfig MediaEGLContext::getConfig(int flags, int version) {
    int renderType = EGL_OPENGL_ES2_BIT;
    if(version >= 3){
//        renderType |= EGL_OPENGL_ES3_BIT_KHR;
    }

    int attribList[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_RENDERABLE_TYPE, renderType,
            EGL_NONE, 0,
            EGL_NONE
    };
    int length = sizeof(attribList) / sizeof(attribList[0]);
    if((flags & FLAG_RECORDABLE) != 0){
        attribList[length - 3] = EGL_RECORDABLE_ANDROID;
        attribList[length - 2] = 1;
    }
    EGLConfig  config = NULL;
    int numConfigs;
    if(!eglChooseConfig(eglDisplay, attribList, &config, 1, &numConfigs)){
        LOGE("unable to find RGB8888 / %d  EGLConfig", version);
        return NULL;
    }
    return config;
}

void MediaEGLContext::release() {
    if(eglDisplay != EGL_NO_DISPLAY){
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }

    if(eglContext != EGL_NO_CONTEXT){
        eglDestroyContext(eglDisplay, eglContext);
    }
    eglDisplay = EGL_NO_DISPLAY;
    eglContext = EGL_NO_CONTEXT;
}


void MediaEGLContext::checkEglError(const char *msg) {
    int error;
    if((error = eglGetError()) != EGL_SUCCESS){
        LOGE("%s: EGL errorL %x", msg, error);
    }
}

EGLContext MediaEGLContext::getContext() {
    return eglContext;
}

void MediaEGLContext::destroy() {
    if(instance){
        std::unique_lock<std::mutex> locks(mutex);
        if(!instance){
            delete instance;
            instance = nullptr;
        }
    }
}