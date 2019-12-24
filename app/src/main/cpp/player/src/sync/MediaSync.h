//
// Created by jackzhous on 2019/8/26.
//

#ifndef MYPLAYER_MEDIASYNC_H
#define MYPLAYER_MEDIASYNC_H

#include <common/PlayerStatus.h>
#include <decoder/VideoDecoder.h>
#include <decoder/AudioDecoder.h>
#include <device/VideoDevice.h>
#include "Thread.h"
#include "MediaClock.h"
extern "C"{
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};

class MediaSync : public Runnable{
    public:
        MediaSync(PlayerStatus* status);
        ~MediaSync();

        void updateExternalClock(double pts);

        virtual void run() override ;

        void stop();

        void reset();

        void start(AudioDecoder* audioDecoder, VideoDecoder* videoDecoder);

        void updateAudioColock(double pts, double time);

        //获取音频时钟漂移
        double getAudioDffClock();

        double getMasterClock();

        MediaClock* getAudioClock();

        MediaClock* getVideoClock();

        MediaClock* getExtClock();

        void setVideoDevice(VideoDevice* device);

        void setMaxFrameDuration(double time);

        void refreshVideoTImer();

        void refreshVideo(double *remain_time);

        void checkExternalClockSpeed();

        void renderVideo();


        private:
            //计算vp的显示时长
            double calculateDuration(JFrame *vp, JFrame *nextvp);

            double calculateDelay(double delay);


        private:
            Mutex mMutex;
            Condition mCond;
            bool abort;
            bool exit;

            MediaClock *audioClock;
            MediaClock *videoClock;
            MediaClock *extClock;
            PlayerStatus* playerStatus;

            Thread* syncThread;
            VideoDecoder* vDecoder;
            AudioDecoder* aDecoder;

            int forceFresh;             //强制刷新
            double maxFrameDuration;    //最大帧演示
            int frameTimerRefresh;      //刷新时钟
            double frameTimer;          //视频时钟,上一帧图像显示的时间戳

            VideoDevice* videoDevice;

            AVFrame* pFrameARGB;
            uint8_t* mBuffer;
            SwsContext* swsContext;
};

#endif //MYPLAYER_MEDIASYNC_H
