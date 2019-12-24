//
// Created by jackzhous on 2019/8/9.
//


#include <jni.h>
#include <cstddef>
#include <AndroidLog.h>
#include "JMediaPlayer.h"
#include <JNIHelp.h>

//因为ffmpeg使用c写的，所以需要使用C方式来编译
extern "C" {
#include <libavcodec/jni.h>
}

static JMediaPlayer* player;

static JavaVM* javaVM = NULL;

const char* JAVA_CLASS = "com/jz/myplayer/player/NativeHelper";


static JNIEnv* getJNIEnv(){
    JNIEnv* env;
    if(javaVM->GetEnv((void **)&env, JNI_VERSION_1_4) < 0){
        return NULL;
    }
    return env;
}

class NativeListener : public JNIListener{
public:
    NativeListener(JNIEnv* env, jobject object, jobject weak_jobject);
    ~NativeListener();
    void notify(int what);

private:
    jclass javaClass;       //java层class
    jmethodID jmethod;      //java曾method
    jobject weakJObject;   //java层class的弱引用
};

void playerInit(JNIEnv* env){
    if(javaVM != NULL){
        LOGI("player init");
    }
}

bool isPlayerError(JNIEnv* env){
    if(player == NULL){
        jniThrowException(env, "java/lang/IllegalStateException");
        LOGE("player error");
        return true;
    }

    return false;
}

static void nativeSetUp(JNIEnv* env, jobject nheler, jobject jplayer, jobject weak_jplayer){
    player = new JMediaPlayer();
    JNIListener* listener = new NativeListener(env, jplayer, weak_jplayer);
    player->setJNIListener(listener);
    player->init();
}

static void setVideoUrl(JNIEnv* env, jobject thiz, jstring str){
    if(str == NULL){
        LOGE("video url is NULL");
        return;
    }
    const char* url = env->GetStringUTFChars(str, 0);
    if(url == NULL){
        LOGE("get video url is NULL");
        return;
    }
    char* url1 = av_strdup(url);
    player->setVideoUrl(url1);
    env->ReleaseStringUTFChars(str, url);
}

static void setVideoPath(JNIEnv* env, jobject thiz, jstring str){
    if(str == NULL){
        LOGE("video url is NULL");
        return;
    }
    const char* url = env->GetStringUTFChars(str, 0);
    if(url == NULL){
        LOGE("get video url is NULL");
        return;
    }
    char* url1 = strdup(url);
    player->setVideoPath(url1);
    env->ReleaseStringUTFChars(str, url);
}

static void preparePlayer(JNIEnv* env, jobject thiz){
    player->prepareAsyn();
}

static int getVideoWidth(JNIEnv* env, jobject thiz){
    return player->getWidth();
}

static int getVideoHeight(JNIEnv* env, jobject thiz){
    return player->getHeight();
}

static int getRorate(JNIEnv* env, jobject thiz){
    return player->getRorate();
}

static void startPlay(JNIEnv* env, jobject thiz){
    player->startPlay();
}

static void onPause(JNIEnv* env, jobject thiz){
    player->onPause();
}

static void onResume(JNIEnv* env, jobject thiz){
    player->onResume();
}

static void onStop(JNIEnv* env, jobject thiz){
    player->onStop();
}


static void release(JNIEnv* env, jobject thiz){
    player->release();
}


static void seekVideo(JNIEnv* env, jobject thiz, jfloat time){
    player->seekVideo(time);
}




static void setLoop(JNIEnv* env, jobject thiz, jint loop){
    player->setLoop(loop);
}

static void setVideoSurface(JNIEnv* env, jobject thiz, jobject surface){
    if(isPlayerError(env)){
        return;
    }
    ANativeWindow *windows = NULL;
    if(surface != NULL){
        windows = ANativeWindow_fromSurface(env, surface);
    }
    player->setSurface(windows);
}



static const JNINativeMethod nativeToJavaMethod[] = {
        {"native_init", "()V", (void *)playerInit},
        {"nativeSetConnect", "(Lcom/jz/myplayer/player/JMediaPlayer;Ljava/lang/Object;)V", (void *)nativeSetUp},
        {"setUrl", "(Ljava/lang/String;)V", (void*)setVideoUrl},
        {"setVideoPath", "(Ljava/lang/String;)V", (void*)setVideoPath},
        {"preparePlayerAsyn", "()V", (void*)preparePlayer},
        {"getVideoWidth","()I", (void*)getVideoWidth},
        {"getVideoHeight", "()I", (void*)getVideoHeight},
        {"getRorate", "()I", (void*)getRorate},
        {"onPause", "()V", (void*)onPause},
        {"onResume", "()V", (void*)onResume},
        {"onStop", "()V", (void*)onStop},
        {"release", "()V", (void*)release},
        {"seekRequest", "(F)V", (void*)seekVideo},
        {"start", "()V", (void*)startPlay},
        {"setLoop", "(I)V", (void*)setLoop},
        {"setVideoSurface", "(Landroid/view/Surface;)V", (void*)setVideoSurface}
};

static int registerJavaMethod(JNIEnv* env){
    int numbersMethod = sizeof(nativeToJavaMethod) / sizeof(nativeToJavaMethod[0]);
    jclass javaclass = env->FindClass(JAVA_CLASS);
    if(javaclass == NULL){
        LOGE("find class %s failed ",JAVA_CLASS);
        return JNI_ERR;
    }
    //返回注册的函数个数
    if(env->RegisterNatives(javaclass, nativeToJavaMethod, numbersMethod) < 0){
        LOGE("register native method failed");
        return JNI_ERR;
    }
    LOGI("register native method success");
    return JNI_OK;
}



NativeListener::NativeListener(JNIEnv* env, jobject object, jobject weak_jobject) {
    jclass jclass1 = env->GetObjectClass(object);
    if(jclass1 == NULL){
        LOGI("find java class error");
        return ;
    }

    javaClass = (jclass)env->NewGlobalRef(jclass1);
    jmethod = env->GetStaticMethodID(javaClass, "handleNativeMsg", "(ILjava/lang/Object;)V");
    weakJObject = env->NewGlobalRef(weak_jobject);

}

NativeListener::~NativeListener() {
    JNIEnv* env = getJNIEnv();
    env->DeleteGlobalRef(javaClass);
    env->DeleteGlobalRef(weakJObject);
}

void NativeListener::notify(int what) {
    JNIEnv* env = getJNIEnv();
    bool status = (javaVM->AttachCurrentThread(&env, NULL) >= 0);

    env->CallStaticVoidMethod(javaClass, jmethod, what, weakJObject);

    if(status)
        javaVM->DetachCurrentThread();
}


extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    av_jni_set_java_vm(vm, NULL);
    javaVM = vm;

    JNIEnv *env = NULL;
    if(vm->GetEnv((void **)&env, JNI_VERSION_1_4) != JNI_OK){
        LOGE("get java env failed ");
        return JNI_ERR;
    }

    if(registerJavaMethod(env) != JNI_OK){
        return JNI_ERR;
    }

    return JNI_VERSION_1_4;
}