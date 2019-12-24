//
// Created by jackzhous on 2019/8/27.
//

#include "AudioDevice.h"

#define OPENSLES_BUFFERS 4 // 最大缓冲区数量
#define OPENSLES_BUFLEN  10 // 缓冲区长度(毫秒)

AudioDevice::AudioDevice() {
    slObject = NULL;
}

AudioDevice::~AudioDevice() {}

/**
 * SLES缓冲回调
 * @param bf
 * @param context
 */
void slBufferPCMCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {

}


int AudioDevice::open(AudioDeviceSpec *input, AudioDeviceSpec *output) {
    SLresult result;
    result = slCreateEngine(&slObject, 0 , NULL, 0, NULL, NULL);
    if(result != SL_RESULT_SUCCESS){
        LOGE("slCreateEngine create engine object failed");
        return -1;
    }

    result = (*slObject)->Realize(slObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS){
        LOGE("Realize engine object failed");
        return -1;
    }

    result = (*slObject)->GetInterface(slObject, SL_IID_ENGINE, &slEngine);
    if(result != SL_RESULT_SUCCESS){
        LOGE("GetInterface engine object failed");
        return -1;
    }

    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*slEngine)->CreateOutputMix(slEngine, &slOutputMixObject, 1, mids,mreq);
    if(result != SL_RESULT_SUCCESS){
        LOGE("(*slEngine)->CreateOutputMix failed");
        return -1;
    }

    result = (*slOutputMixObject)->Realize(slOutputMixObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS){
        LOGE("(*slOutputMixObject)->Realize failed");
        return -1;
    }

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, slOutputMixObject};
    SLDataSink audioSink = {&outputMix, NULL};

    SLDataLocator_AndroidSimpleBufferQueue androidQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            OPENSLES_BUFFERS};

    //根据通道数设置通道mask
    SLuint32 channelMask;
    switch (input->channels){
        case 2:
            channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
            break;

        case 1:
            channelMask = SL_SPEAKER_FRONT_CENTER;
            break;

        default:
            LOGE("%s, invalid channel %d", __func__ , input->channels);
            return -1;
    }

    SLDataFormat_PCM formatPcm = {
        SL_DATAFORMAT_PCM,                  //播放器PCM格式
        input->channels,
        getSLSampleRate(input->freq),       //转换为SL的采样率
        SL_PCMSAMPLEFORMAT_FIXED_16,        //位数
        SL_PCMSAMPLEFORMAT_FIXED_16,
        channelMask,
        SL_BYTEORDER_LITTLEENDIAN           //小端存储
    };

    //配置输入
    SLDataSource slDataSource = {&androidQueue, &formatPcm};

    const SLInterfaceID ids[3] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME, SL_IID_PLAY};
    const SLboolean  req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*slEngine)->CreateAudioPlayer(slEngine, &slPlayerObject, &slDataSource, &audioSink, 3, ids , req);
    if(result != SL_RESULT_SUCCESS){
        LOGE("%s, CreateAudioPlayer failed", __func__);
        return -1;
    }

    result = (*slPlayerObject)->Realize(slPlayerObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS){
        LOGE("%s, CreateAudioPlayer Realize failed", __func__);
        return -1;
    }

    result = (*slPlayerObject)->GetInterface(slPlayerObject, SL_IID_PLAY, &slPlayItf);
    if(result != SL_RESULT_SUCCESS){
        LOGE("%s, CreateAudioPlayer GetInterface player failed", __func__);
        return -1;
    }

    result = (*slPlayerObject)->GetInterface(slPlayerObject, SL_IID_VOLUME, &slVolumeItf);
    if(result != SL_RESULT_SUCCESS){
        LOGE("%s, CreateAudioPlayer GetInterface volume failed", __func__);
        return -1;
    }

    result = (*slPlayerObject)->GetInterface(slPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &slBufferQueueItf);
    if(result != SL_RESULT_SUCCESS){
        LOGE("%s, CreateAudioPlayer GetInterface android simple buffer failed", __func__);
        return -1;
    }

    result = (*slBufferQueueItf)->RegisterCallback(slBufferQueueItf, slBufferPCMCallBack, this);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("%s: slBufferQueueItf->RegisterCallback() failed", __func__);
        return -1;
    }

    //计算缓冲区大小
    bytes_per_frame = formatPcm.numChannels * formatPcm.bitsPerSample / 8;
    milli_per_buffer = OPENSLES_BUFLEN;
    frames_per_buffer = milli_per_buffer * formatPcm.samplesPerSec / 1000000;       //为什么要除1000000而不是1000
    bytes_per_buffer = bytes_per_frame * frames_per_buffer;
    buffer_capacity = OPENSLES_BUFFERS * bytes_per_buffer;

    LOGI("OpenSL-ES: bytes_per_frame  = %d bytes\n",  bytes_per_frame);
    LOGI("OpenSL-ES: milli_per_buffer = %d ms\n",     milli_per_buffer);
    LOGI("OpenSL-ES: frame_per_buffer = %d frames\n", frames_per_buffer);
    LOGI("OpenSL-ES: buffer_capacity  = %d bytes\n",  buffer_capacity);
    LOGI("OpenSL-ES: buffer_capacity  = %d bytes\n",  (int)buffer_capacity);

    if(output != NULL){
        *output = *input;
        output->size = (uint32_t)buffer_capacity;
        output->freq = formatPcm.samplesPerSec / 1000;
    }
    audioDeviceSpec = *input;

    //创建缓冲区
    buffer = (uint8_t*)malloc(buffer_capacity);
    if(!buffer){
        LOGE("%s: failed to alloc buffer %d\n", __func__, (int)buffer_capacity);
        return -1;
    }

    memset(buffer, 0, buffer_capacity);
    for(int i = 0; i < OPENSLES_BUFFERS; i++){
        result = (*slBufferQueueItf)->Enqueue(slBufferQueueItf, buffer + i * bytes_per_buffer, bytes_per_buffer);
        if(result != SL_RESULT_SUCCESS){
            LOGE("%s: slBufferQueueItf->Enqueue(000...) failed", __func__);
        }
    }

    return buffer_capacity;
}



SLuint32 AudioDevice::getSLSampleRate(int sampleRate) {
    switch (sampleRate) {
        case 8000: {
            return SL_SAMPLINGRATE_8;
        }
        case 11025: {
            return SL_SAMPLINGRATE_11_025;
        }
        case 12000: {
            return SL_SAMPLINGRATE_12;
        }
        case 16000: {
            return SL_SAMPLINGRATE_16;
        }
        case 22050: {
            return SL_SAMPLINGRATE_22_05;
        }
        case 24000: {
            return SL_SAMPLINGRATE_24;
        }
        case 32000: {
            return SL_SAMPLINGRATE_32;
        }
        case 44100: {
            return SL_SAMPLINGRATE_44_1;
        }
        case 48000: {
            return SL_SAMPLINGRATE_48;
        }
        case 64000: {
            return SL_SAMPLINGRATE_64;
        }
        case 88200: {
            return SL_SAMPLINGRATE_88_2;
        }
        case 96000: {
            return SL_SAMPLINGRATE_96;
        }
        case 192000: {
            return SL_SAMPLINGRATE_192;
        }
        default: {
            return SL_SAMPLINGRATE_44_1;
        }
    }
}


void AudioDevice::run() {
    LOGI("sl es 音频设备开始工作");
    uint8_t *nextBuffer = NULL;
    int nextBufferIndex = 0;
    //启动开始播放
    if(!abortRequest && !pauseRequest){
        (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);
    }

    while (true){
        if(abortRequest){
            break;
        }


        if(pauseRequest){
            break;
        }
        //获取缓冲区队列状态
        SLAndroidSimpleBufferQueueState slState = {0};
        SLresult slRet = (*slBufferQueueItf)->GetState(slBufferQueueItf, &slState);
        if(slRet != SL_RESULT_SUCCESS){
            LOGE("%s: slBufferQueueItf->GetState failed", __func__);
            mMutex.unlock();
        }

        //判断队列缓冲区是否填满了
        mMutex.lock();
        if(!abortRequest && (pauseRequest || slState.count >= OPENSLES_BUFFERS)){
            while (!abortRequest && (pauseRequest || slState.count >= OPENSLES_BUFFERS)){
                if(!pauseRequest){
                    (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);
                }
                mCond.waitRelative(mMutex, 10 * 1000000);
                slRet = (*slBufferQueueItf)->GetState(slBufferQueueItf, &slState);
                if(slRet != SL_RESULT_SUCCESS){
                    LOGE("%s: slBufferQueueItf->GetState failed", __func__);
                    mMutex.unlock();
                }

                if(pauseRequest){
                    (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PAUSED);
                }
            }

            if (!abortRequest && !pauseRequest) {
                (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);
            }
        }
        if(flushRequest){
            (*slBufferQueueItf)->Clear(slBufferQueueItf);
            flushRequest = 0;
        }
        mMutex.unlock();

        //缓冲区数据填充
        mMutex.lock();
        if(audioDeviceSpec.callback != NULL){
            nextBuffer = buffer + nextBufferIndex * bytes_per_buffer;
            nextBufferIndex = (nextBufferIndex+1) % OPENSLES_BUFFERS;
            audioDeviceSpec.callback(audioDeviceSpec.userData, nextBuffer, bytes_per_buffer);
        }
        mMutex.unlock();

        //更新音量
        if(updateVolume){
            if(slVolumeItf != NULL){
                SLmillibel level = getAmplificationLevel((leftVolume + rightVolume) / 2);
                SLresult lresult = (*slVolumeItf)->SetVolumeLevel(slVolumeItf, level);
                if (lresult != SL_RESULT_SUCCESS) {
                    LOGE("slVolumeItf->SetVolumeLevel failed %d\n", (int)lresult);
                }
            }
            updateVolume = false;
        }

        if(flushRequest){
            (*slBufferQueueItf)->Clear(slBufferQueueItf);
            flushRequest = 0;
        } else{
            if(slPlayItf){
                (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);
            }
            slRet = (*slBufferQueueItf)->Enqueue(slBufferQueueItf, nextBuffer, bytes_per_buffer);
            if (slRet == SL_RESULT_SUCCESS){

            } else if(slRet == SL_RESULT_BUFFER_INSUFFICIENT){
                LOGE("SL_RESULT_BUFFER_INSUFFICIENT\n");
            } else{
                LOGE("slBufferQueueItf->Enqueue() = %d\n", (int)slRet);
                break;
            }
        }
    }

    if(slPlayItf){
        (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_STOPPED);
    }
}


void AudioDevice::start() {
    //回调存在时，标明成功打开音频设备
    if(audioDeviceSpec.callback != NULL){
        abortRequest = 0;
        pauseRequest = 0;
        if(!audioThread){
            audioThread = new Thread(this, Priority_High);
            audioThread->start();
            LOGI("start audio device Thread");
        } else
            LOGE("audio device is not open success");
    } else{
        LOGE("audio device is not open success");
    }
}

void AudioDevice::stop() {
    mMutex.lock();
    abortRequest = 1;
    mCond.signal();
    mMutex.unlock();

    if(audioThread){
        audioThread->join();
        delete audioThread;
        audioThread = NULL;
    }
}

void AudioDevice::pause() {
    mMutex.lock();
    pauseRequest = 1;
    mCond.signal();
    mMutex.unlock();
}

void AudioDevice::resume() {
    mMutex.lock();
    pauseRequest = 0;
    mCond.signal();
    mMutex.unlock();
}

/**
 * 清空缓冲队列
 */
void AudioDevice::flush() {
    mMutex.lock();
    flushRequest = 1;
    mCond.signal();
    mMutex.unlock();
}


SLmillibel AudioDevice::getAmplificationLevel(float volumeLevel) {
    if(volumeLevel < 0.0000001){
        return SL_MILLIBEL_MIN;
    }
    /**
     * lround四舍五入取整 返回long int型
     *
     */
    SLmillibel mb = lround(20.0f * log10f(volumeLevel));
    if(mb < SL_MILLIBEL_MIN){
        mb = SL_MILLIBEL_MIN;
    }
    return mb;
}