//
// Created by jackzhous on 2019/8/9.
//

#ifndef MYPLAYER_JMEDIAPLAYER_H
#define MYPLAYER_JMEDIAPLAYER_H


#include <common/MessageQueue.h>
#include <player/Player.h>
#include "Thread.h"
#include "common/Config.h"


class JNIListener{
public:
    virtual void notify(int what) = 0;     //回调到java层的接口
};

class JMediaPlayer : public Runnable{
    public:
        JMediaPlayer();
        virtual ~JMediaPlayer();
        void setJNIListener(JNIListener* listener);
        void notifyJava(int what);
        void setVideoUrl(char* url);
        void setVideoPath(char* path);
        void prepareAsyn();
        int getWidth();
        int getHeight();
        int getRorate();
        void onResume();
        void onStop();
        void init();
        void release();
        void reset();
        void startPlay();
        void onPause();
        void seekVideo(float time);
        void setLoop(int loop);
        void setSurface(ANativeWindow* window);

    protected:
        virtual void run();

    private:
        void freeJNIMessage(JNIMessage* msg);


    private:
        JNIListener* listener;
        Player* player;
        bool abort;      //停止标志
        bool seeking;      //定位中
        Thread* msgThread;
        VideoDevice* vDevice;
        Mutex mMutex;
        Condition mCond;
};


#endif //MYPLAYER_JMEDIAPLAYER_H