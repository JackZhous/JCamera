//
// Created by jackzhous on 2019/12/24.
//
#include <jni.h>
#include <cstddef>
#include <AndroidLog.h>
#include <Recorder.h>
#include <assert.h>

//因为ffmpeg使用c写的，所以需要使用C方式来编译
extern "C" {
#include <libavcodec/jni.h>
}
static JavaVM* javaVM = NULL;

const char* JAVA_CLASS = "com/jz/jcamera/recorder/FFmpegRecorder";


static JNIEnv *getJNIEnv() {
    JNIEnv *env;
    assert(javaVM != nullptr);
    if (javaVM->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return nullptr;
    }
    return env;
}

class MyJNIListener : public OnRecordLisenter{
public:
    MyJNIListener(JavaVM *vm, JNIEnv *env, jobject listener);

    virtual ~MyJNIListener();

    void onRecordStart() override;

    void onRecording(float duration) override ;

    void onRecordFinish(bool success, float d) override ;

    void onRecordError(const char *msg) override;
private:
    MyJNIListener();

public:
    JavaVM *jniVM;
    jobject javaListener;
    jmethodID jl_onRecordStart;
    jmethodID jl_onRecording;
    jmethodID jl_onRecordFinish;
    jmethodID jl_onRecordError;
};

MyJNIListener::MyJNIListener(JavaVM *vm, JNIEnv *env, jobject listener) {
    jniVM = vm;
    if(listener != nullptr){
        javaListener = env->NewGlobalRef(listener);
    } else{
        javaListener = nullptr;
    }
    jclass myClass = env->GetObjectClass(listener);
    if(myClass != nullptr){
        jl_onRecordStart = env->GetMethodID(myClass, "onRecordStart", "()V");
        jl_onRecording = env->GetMethodID(myClass, "onRecording", "(F)V");
        jl_onRecordFinish = env->GetMethodID(myClass, "onRecordFinish", "(ZF)V");
        jl_onRecordError = env->GetMethodID(myClass, "onRecordError", "(Ljava/lang/String;)V");
    }
}

MyJNIListener::~MyJNIListener() {
    if(javaListener != nullptr){
        JNIEnv *env = getJNIEnv();
        env->DeleteGlobalRef(javaListener);
        javaListener = nullptr;
        jniVM = nullptr;
    }
}

void MyJNIListener::onRecordStart(){
    if(jl_onRecordStart != nullptr){
        JNIEnv *env;
        if(jniVM->AttachCurrentThread(&env, nullptr) != JNI_OK){
            return;
        }
        env->CallVoidMethod(javaListener, jl_onRecordStart);
        jniVM->DetachCurrentThread();
    }
}

void MyJNIListener::onRecording(float duration) {
    if(jl_onRecording != nullptr){
        JNIEnv *env;
        if(jniVM->AttachCurrentThread(&env, nullptr) != JNI_OK){
            return;
        }
        env->CallVoidMethod(javaListener, jl_onRecording, duration);
        jniVM->DetachCurrentThread();
    }
}

void MyJNIListener::onRecordFinish(bool success, float d) {
    if(jl_onRecordFinish != nullptr){
        JNIEnv *env;
        if(jniVM->AttachCurrentThread(&env, nullptr) != JNI_OK){
            return;
        }
        env->CallVoidMethod(javaListener, jl_onRecordFinish, success, d);
        jniVM->DetachCurrentThread();
    }
}

void MyJNIListener::onRecordError(const char *msg) {
    if(jl_onRecordError != nullptr){
        JNIEnv *env;
        if(jniVM->AttachCurrentThread(&env, nullptr) != JNI_OK){
            return;
        }
        jstring jmsg = nullptr;
        if(msg != nullptr){
            jmsg = env->NewStringUTF(msg);
        } else{
            jmsg = env->NewStringUTF("unknown");
        }
        env->CallVoidMethod(javaListener, jl_onRecordError, jmsg);
        jniVM->DetachCurrentThread();
    }
}


jlong nativeInit(){
    Recorder* recorder = new Recorder();
    return (jlong)recorder;
}

void setRecordListener(JNIEnv* env, jobject thiz, jlong point, jobject listener){
    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr){
        MyJNIListener* myLis = new MyJNIListener(javaVM, env, listener);
        recorder->setMRecordListener(myLis);
    }
}

void setDstUrl(JNIEnv* env, jobject thiz, jlong point, jstring dstUrl){

    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr){
        const char *url = env->GetStringUTFChars(dstUrl, nullptr);
        RecordParams* params = recorder->getRecordParams();
        params->setOutput(url);
        env->ReleaseStringUTFChars(dstUrl, url);
    }
}

void setVideoParams(JNIEnv* env, jobject thiz, jlong point, jint w, jint h, jint frameRate,
                            jint pixelFormat, jlong maxbit, jint qulaty){
    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr){
        RecordParams *params = recorder->getRecordParams();
        params->setVideoParams(w, h, frameRate,pixelFormat, maxbit, qulaty);
    }
}

void setAudioparams(JNIEnv* env, jobject thiz, jlong point, jint sampleRate, jint sampleFormat,
                            jint channels){
    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr){
        RecordParams *params = recorder->getRecordParams();
        params->setAudioParams(sampleRate, sampleFormat, channels);
    }
}

void setRotate(JNIEnv* env, jobject thiz, jlong point, jint rotate){
    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr){
        RecordParams *params = recorder->getRecordParams();
        params->setRotate(rotate);
    }
}

void setAudioEncoder(JNIEnv* env, jobject thiz, jlong point, jstring encoder){
    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr){
        const char *coder = env->GetStringUTFChars(encoder, nullptr);
        RecordParams *params = recorder->getRecordParams();
        params->setAudioEncoder(coder);
        env->ReleaseStringUTFChars(encoder, coder);
    }
}

void setVideoEncoder(JNIEnv* env, jobject thiz, jlong point, jstring encoder){
    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr){
        const char *coder = env->GetStringUTFChars(encoder, nullptr);
        RecordParams *params = recorder->getRecordParams();
        params->setVideoEncoder(coder);
        env->ReleaseStringUTFChars(encoder, coder);
    }
}

void setAudioFilter(JNIEnv* env, jobject thiz, jlong point, jstring encoder){
    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr){
        const char *coder = env->GetStringUTFChars(encoder, nullptr);
        RecordParams *params = recorder->getRecordParams();
        params->setAudioFilter(coder);
        env->ReleaseStringUTFChars(encoder, coder);
    }
}

void setVideoFilter(JNIEnv* env, jobject thiz, jlong point, jstring encoder){
    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr){
        const char *coder = env->GetStringUTFChars(encoder, nullptr);
        RecordParams *params = recorder->getRecordParams();
        params->setVideoFilter(coder);
        env->ReleaseStringUTFChars(encoder, coder);
    }
}

void release(JNIEnv* env, jobject thiz, jlong point){
    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr){
        recorder->release();
    }
}

void startRecord(JNIEnv* env, jobject thiz, jlong point){
    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr){
        int ret = recorder->prepare();
        if(ret < 0){
            LOGE("failed to prepare record");
        } else{
            recorder->startRecord();
        }
    }
}

void stopRecord(JNIEnv* env, jobject thiz, jlong point){
    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr){
        recorder->stopRecord();
    }
}

void recordAudioFrame(JNIEnv* env, jobject thiz, jlong point, jbyteArray jdata, jint length){
    LOGI("got audio");
    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr && recorder->isRecording()){
        uint8_t *pcmData = (uint8_t*)malloc((size_t)length);
        if(pcmData == nullptr){
            LOGE("could not allocate memory");
            return;
        }
        jbyte *data = env->GetByteArrayElements(jdata, nullptr);
        memcpy(pcmData, data, (size_t)length);
        env->ReleaseByteArrayElements(jdata, data, 0);

        auto media = new AVMediaData();
        media->setAudio(pcmData, length);
        media->setPts(getCurrentTimeMs());
        recorder->recordFrame(media);
    }
}

void recordVideoFrame(JNIEnv* env, jobject thiz, jlong point, jbyteArray jdata, jint length,
                                                            jint w, jint h, jint format){
    Recorder* recorder = (Recorder*)point;
    if(recorder != nullptr && recorder->isRecording()){
        uint8_t *pcmData = (uint8_t*)malloc((size_t)length);
        if(pcmData == nullptr){
            LOGE("could not allocate memory");
            return;
        }
        jbyte *data = env->GetByteArrayElements(jdata, nullptr);
        memcpy(pcmData, data, (size_t)length);
        env->ReleaseByteArrayElements(jdata, data, 0);

        auto media = new AVMediaData();
        media->setVideo(pcmData, length, w, h, format);
        media->setPts(getCurrentTimeMs());
        recorder->recordFrame(media);
    }
}




static const JNINativeMethod nativeToJavaMethod[] = {
        {"nativeInit", "()J", (void *)nativeInit},
//        {"setOutputs", "()V",(void*)setDstUrl},
        {"setOutputs", "(JLjava/lang/String;)V",(void*)setDstUrl},
        {"_setRecordListener", "(JLcom/jz/jcamera/controller/OnRecordListener;)V", (void*)setRecordListener},
        {"_setVideoParams", "(JIIIIJI)V", (void*)setVideoParams},
        {"_setRotate", "(JI)V", (void*)setRotate},
        {"_setAudioParams", "(JIII)V", (void*)setAudioparams},
        {"_setAudioEncoder", "(JLjava/lang/String;)V", (void*)setAudioEncoder},
        {"_setVideoEncoder", "(JLjava/lang/String;)V", (void*)setVideoEncoder},
        {"_setAudioFilter", "(JLjava/lang/String;)V", (void*)setAudioFilter},
        {"_setVideoFilter", "(JLjava/lang/String;)V", (void*)setVideoFilter},
        {"_release", "(J)V", (void*)release},
        {"_startRecord", "(J)V", (void*)startRecord},
        {"_stopRecord", "(J)V", (void*)stopRecord},
        {"_recordAudioFrame", "(J[BI)V", (void*)recordAudioFrame},
        {"_recordVideoFrame", "(J[BIIII)V", (void*)recordVideoFrame}
};

int registerJavaMethod(JNIEnv* env){
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
