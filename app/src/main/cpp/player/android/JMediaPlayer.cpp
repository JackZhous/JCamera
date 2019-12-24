//
// Created by jackzhous on 2019/8/9.
//

#include "JMediaPlayer.h"

JMediaPlayer::JMediaPlayer() {
    listener = NULL;
    abort = false;
    seeking = false;
    msgThread = NULL;
    vDevice = NULL;
    player = NULL;
}

JMediaPlayer::~JMediaPlayer() {

}

void JMediaPlayer::init() {
    mMutex.lock();
    abort = false;
    mCond.signal();

    if(vDevice == NULL){
        vDevice = new VideoDevice();
    }

    if(msgThread == NULL){
        msgThread = new Thread(this);
    }
    msgThread->start();

    mMutex.unlock();
}

void JMediaPlayer::reset() {
    if(player != NULL){
        player->reset();
        delete player;
        player = NULL;
    }
}

void JMediaPlayer::setSurface(ANativeWindow *window) {
    if(vDevice != NULL){
        vDevice->surfaceCreate(window);
    }
}


void JMediaPlayer::setJNIListener(JNIListener *listener) {
    this->listener = listener;
}

void JMediaPlayer::notifyJava(int what) {
    if(listener != NULL){
        listener->notify(what);
    }
}

void JMediaPlayer::onPause() {
    if(player != NULL){
        player->pause();
    }
}


void JMediaPlayer::setVideoUrl(char* url) {
    if(url == NULL){
        return;
    }
    player = new Player();
    player->setVideoDevice(vDevice);
    player->setVideoUrl(url);
}

void JMediaPlayer::setVideoPath(char *path) {
    if(path == NULL){
        return;
    }
    player = new Player();
    player->setVideoDevice(vDevice);
    player->setVideoPath(path);
}

void JMediaPlayer::run() {
    int ret;
    for(;;){
        if(abort){
            LOGI("handle message abort!");
            break;
        }

        JNIMessage msg;
        if(!player || !player->getMessageQueue()){
            av_usleep(10 * 1000);
            continue;
        }

        ret = player->getMessageQueue()->getMessage(&msg);
        if(ret < 0){
            LOGE("get handle message failed!");
        }
        LOGI("progress %s", msg.message);
        switch (msg.what){
            case SEEK_END:
                seeking = false;
                break;
        }
        notifyJava(msg.what);
        freeJNIMessage(&msg);
    }

}


void JMediaPlayer::freeJNIMessage(JNIMessage *msg) {
    if(msg == NULL){
        return;
    }
    if(msg->message != NULL){
        av_free(msg->message);
    }
}

void JMediaPlayer::prepareAsyn() {
    if(player != NULL){
        player->prepare();
    }
}

int JMediaPlayer::getHeight() {
    if(player != NULL){
        return player->getHeight();
    }
    return 0;
}

void JMediaPlayer::onStop() {
    if(player != NULL){
        return player->stop();
    }
}

int JMediaPlayer::getRorate() {
    if(player != NULL){
        return player->getRorate();
    }
    return 0;
}

void JMediaPlayer::startPlay() {
    if(player != NULL){
        player->startPlay();
    }
}

void JMediaPlayer::onResume() {
    if(player != NULL){
        player->onResume();
    }
}

int JMediaPlayer::getWidth() {
    if(player != NULL){
        return player->getWidth();
    }
    return 0;
}

void JMediaPlayer::seekVideo(float time) {
    if(player != NULL){
        if(seeking){
            return;
        } else{
            player->seekVideo(time);
            seeking = true;
        }
    }
}

void JMediaPlayer::setLoop(int loop) {
    if(player != NULL){
        player->setLoop(loop);
    }
}

void JMediaPlayer::release() {
    mMutex.lock();
    abort = true;
    mCond.signal();
    mMutex.unlock();

    reset();

    if(msgThread != NULL){
        msgThread->join();
        delete msgThread;
        msgThread = NULL;
    }

    if(vDevice != NULL){
        delete  vDevice;
        vDevice = NULL;
    }

    if(listener != NULL){
        delete listener;
        listener = NULL;
    }
}
