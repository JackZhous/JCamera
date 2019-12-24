//
// Created by jackzhous on 2019/8/28.
//

#ifndef MYPLAYER_MEDIACLOCK_H
#define MYPLAYER_MEDIACLOCK_H

#include <cmath>
#include <common/Config.h>

extern "C"{
    #include <libavutil/time.h>
};

class MediaClock{
public:
    MediaClock();
    ~MediaClock();
    void init();
    double getClock();
    void setClock(double pts, double time);
    void setClock(double pts);
    void setSpeed(double speed);
    double getSpeed();

    //同步到从属时钟
    void syncToSlave(MediaClock* clock);

private:

    //以下是秒
    double pts;
    double ptd_drift;       //漂移时间，即时间差
    double last_update;
    double speed;           //播放速度
    int paused;
};

#endif //MYPLAYER_MEDIACLOCK_H
