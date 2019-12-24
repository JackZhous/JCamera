//
// Created by jackzhous on 2019-09-11.
//

#include "EGLHelper.h"


EGLHelper::EGLHelper() {
    mEglDisplay = EGL_NO_DISPLAY;
    mEglConfig = NULL;
    mEglContext = EGL_NO_CONTEXT;
    mEglVersion = -1;

    //设置时间戳方法，只有Android存在这个方法
    eglPresentationTimeANDROID = NULL;
}

EGLHelper::~EGLHelper() {
    release();
}

bool EGLHelper::init(int flags) {
    return init(MediaEGLContext::getInstance()->getContext(), flags);
}

bool EGLHelper::init(EGLContext shareContext, int flags) {
    if(mEglDisplay != EGL_NO_DISPLAY){
        LOGE("EGLDisplay is already set up");
        return false;
    }

    if(shareContext == NULL){
        LOGE("egl share context is null");
        shareContext = EGL_NO_CONTEXT;
    } else{
        LOGI("egl share context is created");
    }

    mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(mEglDisplay == EGL_NO_DISPLAY){
        LOGE("unable to create eglDisplay");
        return false;
    }
    // 初始化mEGLDisplay param2 主版本号  param3 次版本号
    if(!eglInitialize(mEglDisplay, 0, 0)){
        LOGE("unable init eglDisplay");
        mEglDisplay = EGL_NO_DISPLAY;
        return false;
    }

    // 判断是否尝试使用GLES3
    if ((flags & FLAG_TRY_GLES3) != 0) {
        EGLConfig config = getConfig(flags, 3);
        if (config != NULL) {
            int attrib3_list[] = {
                    EGL_CONTEXT_CLIENT_VERSION, 3,
                    EGL_NONE
            };
            //sharesdk会和返回的context绑定在一块，sharedContext可以在多个线程之间共享
            EGLContext context = eglCreateContext(mEglDisplay, config, shareContext, attrib3_list);
            checkEglError("eglCreateContext");
            if (eglGetError() == EGL_SUCCESS) {
                mEglConfig = config;
                mEglContext = context;
                mEglVersion = 3;
            }
        }
    }

    // 判断如果GLES3的EGLContext没有获取到，则尝试使用GLES2
    if (mEglContext == EGL_NO_CONTEXT) {
        EGLConfig config = getConfig(flags, 2);
        int attrib2_list[] = {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_NONE
        };
        EGLContext context = eglCreateContext(mEglDisplay, config, shareContext, attrib2_list);
        checkEglError("eglCreateContext");
        if (eglGetError() == EGL_SUCCESS) {
            mEglConfig = config;
            mEglContext = context;
            mEglVersion = 2;
        }
    }

#if defined(__ANDROID__)
    // 获取eglPresentationTimeANDROID方法的地址,此种办法拿到api
    eglPresentationTimeANDROID = (EGL_PRESENTATION_TIME_ANDROIDPROC)
            eglGetProcAddress("eglPresentationTimeANDROID");
    if (!eglPresentationTimeANDROID) {
        LOGE("eglPresentationTimeANDROID is not available!");
    }
#endif

    int values[1] = {0};
    //查询版本信息
    eglQueryContext(mEglDisplay, mEglContext, EGL_CONTEXT_CLIENT_VERSION, values);
    LOGW("EGLContext created, client version %d", values[0]);
    return true;


}

EGLConfig EGLHelper::getConfig(int flags, int version) {
    int renderableType = EGL_OPENGL_ES2_BIT;
    if (version >= 3) {
//        renderableType |= EGL_OPENGL_ES3_BIT_KHR;
    }
    //EGLConfig是配置surface的一些配置信息,如下配置EGL_RED_SIZE 红色用8bit来保存
    int attribList[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            //EGL_DEPTH_SIZE, 16,
            //EGL_STENCIL_SIZE, 8,
            EGL_RENDERABLE_TYPE, renderableType,
            EGL_NONE, 0,      // placeholder for recordable [@-3]
            EGL_NONE
    };
    int length = sizeof(attribList) / sizeof(attribList[0]);
    if ((flags & FLAG_RECORDABLE) != 0) {
        attribList[length - 3] = EGL_RECORDABLE_ANDROID;
        attribList[length - 2] = 1;
    }
    EGLConfig configs = NULL;
    int numConfigs;
    //参数1限制返回的configs数量为1， numConfigs总共的configs数量
    if (!eglChooseConfig(mEglDisplay, attribList, &configs, 1, &numConfigs)) {
        LOGW("unable to find RGB8888 / %d  EGLConfig", version);
        return NULL;
    }
    return configs;
}

void EGLHelper::release() {
    if (mEglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    if (mEglContext != EGL_NO_CONTEXT) {
        eglDestroyContext(mEglDisplay, mEglContext);
    }
    if (mEglDisplay != EGL_NO_DISPLAY) {
        eglReleaseThread();
        eglTerminate(mEglDisplay);
    }
    mEglDisplay = EGL_NO_DISPLAY;
    mEglConfig = NULL;
    mEglContext = EGL_NO_CONTEXT;
}

EGLContext EGLHelper::getEglContext() {
    return mEglContext;
}


EGLSurface EGLHelper::createSurface(ANativeWindow *surface) {
    if (surface == NULL) {
        LOGE("Window surface is NULL!");
        return NULL;
    }
    int attrib_list[] = {
            EGL_NONE
    };
    EGLSurface eglSurface = eglCreateWindowSurface(mEglDisplay, mEglConfig, surface, attrib_list);
    checkEglError("eglCreateWindowSurface");
    if (eglSurface == EGL_NO_SURFACE) {
        LOGE("EGLSurface was null");
    }
    return eglSurface;
}

EGLSurface EGLHelper::createSurface(int width, int height) {
    int attrib_list[] = {
            EGL_WIDTH, width,
            EGL_HEIGHT, height,
            EGL_NONE
    };
    EGLSurface eglSurface = eglCreatePbufferSurface(mEglDisplay, mEglConfig, attrib_list);
    checkEglError("eglCreatePbufferSurface");
    if (eglSurface == EGL_NO_SURFACE) {
        LOGE("EGLSurface was null");
    }
    return eglSurface;
}


void EGLHelper::makeCurrent(EGLSurface eglSurface) {
    if (mEglDisplay == EGL_NO_DISPLAY) {
        LOGE("Note: makeCurrent w/o display.\n");
    }

    if (!eglMakeCurrent(mEglDisplay, eglSurface, eglSurface, mEglContext)) {
        // 出错处理
    }
}

void EGLHelper::makeCurrent(EGLSurface drawSurface, EGLSurface readSurface) {
    if (mEglDisplay == EGL_NO_DISPLAY) {
        LOGE("Note: makeCurrent w/o display.\n");
    }
    if (!eglMakeCurrent(mEglDisplay, drawSurface, readSurface, mEglContext)) {
        // 出错处理
    }
}

void EGLHelper::makeNothingCurrent() {
    if (!eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        // 出错处理
    }
}

int EGLHelper::swapBuffers(EGLSurface eglSurface) {
    if (!eglSwapBuffers(mEglDisplay, eglSurface)) {
        return eglGetError();
    }
    return EGL_SUCCESS;
}

#if defined(__ANDROID__)
void EGLHelper::setPresentationTime(EGLSurface eglSurface, long nsecs) {
    if (eglPresentationTimeANDROID != NULL) {
        eglPresentationTimeANDROID(mEglDisplay, eglSurface, nsecs);
    }
}
#endif

bool EGLHelper::isCurrent(EGLSurface eglSurface) {
    return (mEglContext == eglGetCurrentContext())
           && (eglSurface == eglGetCurrentSurface(EGL_DRAW));
}

int EGLHelper::querySurface(EGLSurface eglSurface, int what) {
    int value;
    eglQuerySurface(mEglContext, eglSurface, what, &value);
    return value;
}

const char *EGLHelper::queryString(int what) {
    return eglQueryString(mEglDisplay, what);
}

int EGLHelper::getEGLVersion() {
    return mEglVersion;
}

void EGLHelper::checkEglError(const char *msg) {
    int error;
    if ((error = eglGetError()) != EGL_SUCCESS) {
        LOGE("%s: EGL error: %x", msg, error);
    }
}

void EGLHelper::destroySurface(EGLSurface eglSurface) {
    eglDestroySurface(mEglDisplay, eglSurface);
}