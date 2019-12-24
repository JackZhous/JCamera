//
// Created by jackzhous on 2019/8/13.
//

#ifndef MYPLAYER_PLAYERSTATUS_H
#define MYPLAYER_PLAYERSTATUS_H

#include "stdint.h"
#include "MessageQueue.h"

extern "C"{
    #include <libavutil/dict.h>
    #include <libavutil/avutil.h>
};

typedef enum {
    AV_SYNC_AUDIO,      // 同步到音频时钟
    AV_SYNC_VIDEO,      // 同步到视频时钟
    AV_SYNC_EXTERNAL,   // 同步到外部时钟
} SyncType;

class PlayerStatus{
    public:
        PlayerStatus();
        ~PlayerStatus();

        void setVideoUrl(char* url);
        void setVideoPath(char* path);
        void reset();

    public:
        char* url;                      //直播地址
        char* vedioPath;                //视频地址
        AVDictionary* format_options;
        char abortRequest;               //停止请求
        char pauseRequest;
        char* forcedVideoCodecName;
        char* forcedAduioCodecName;
        int64_t mDuration;
        int64_t playDuration;           //播放时长
        bool isLiveStream;
        int seekRequest;
        int64_t seekPos;                //视频位置
        int seekFlag;
        int loop;                       //循环播放
        int64_t start_time;             //开始时间
        MessageQueue* queue;
        int reorderVideoPts;            //是否需要重排
        SyncType syncType;

        int mute;                       // 静音播放
    int lowres;                     // 解码上下文的lowres标志
    int fast;                       // 解码上下文的AV_CODEC_FLAG2_FAST标志
};

#endif //MYPLAYER_PLAYERSTATUS_H
