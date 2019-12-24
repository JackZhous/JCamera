//
// Created by jackzhous on 2019/8/28.
//

#include "MediaClock.h"


MediaClock::MediaClock() {
    init();
}


MediaClock::~MediaClock() {}

double MediaClock::getClock() {
    if(paused){
        return pts;
    } else{
        //等比例计算时间
        double time = av_gettime_relative() / 1000000.0;
        return ptd_drift + time - (time - last_update) * (1.0 - speed);
    }
}

double MediaClock::getSpeed() {
    return speed;
}

void MediaClock::init() {
    paused = 0;
    speed = 1.0;
    setClock(NAN);
}

void MediaClock::setClock(double pts) {
    double time = av_gettime_relative() / 1000000.0;
    setClock(pts, time);
}

void MediaClock::setClock(double pts, double time) {
    this->pts = pts;
    last_update = time;
    ptd_drift = pts - time;
}

void MediaClock::setSpeed(double speed) {
    setClock(getClock());
    this->speed  = speed;
}

void MediaClock::syncToSlave(MediaClock *clock) {
    double clocks = getClock();
    double slaveClock = clock->getClock();
    if(!isnan(slaveClock) && (isnan(clocks) || fabs(clocks - slaveClock) > AV_NOSYNC_THRESHOLD)){
        setClock(slaveClock);
    }
}