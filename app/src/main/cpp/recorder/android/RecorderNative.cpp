//
// Created by jackzhous on 2019/12/24.
//
#include <jni.h>
#include <cstddef>
#include <AndroidLog.h>
#include <JNIHelp.h>
#include <Recorder.h>

//因为ffmpeg使用c写的，所以需要使用C方式来编译
extern "C" {
#include <libavcodec/jni.h>
}


static JavaVM* javaVM = NULL;

const char* JAVA_CLASS = "com/jz/myplayer/player/NativeHelper";


jlong nativeInit(){
    Recorder* recorder = new Recorder();
    return (jlong)recorder;
}


static const JNINativeMethod nativeToJavaMethod[] = {
        {"nativeInit", "()J", (void *)nativeInit}
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
