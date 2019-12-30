//
// Created by jackzhous on 2019/8/9.
//

#ifndef MYPLAYER_CONDITION_H
#define MYPLAYER_CONDITION_H


#include "Mutex.h"
#include <time.h>
#include <stdint.h>

class Condition{
    public:
        Condition();
        ~Condition();
        Condition(int type);

        enum {
            PRIVATE = 0,
            SHARED = 1
        };

        enum WakeUpType{
            WAKE_UP_ONE = 0,
            WAKE_UP_ALL = 1
        };

        int wait(Mutex& mutex);
        int waitRelative(Mutex& mutex, int64_t time);

        void signal();

        void signalAll(WakeUpType type){
            if(type == WAKE_UP_ONE){
                signal();
            } else{
                broadcast();
            }
        };
        void broadcast();

    private:
        pthread_cond_t cond;
};


inline Condition::Condition() {
    pthread_cond_init(&cond, NULL);
}

inline Condition::~Condition() {
    pthread_cond_destroy(&cond);
}

inline Condition::Condition(int type) {
    if(type == SHARED){
        pthread_condattr_t attr;
        pthread_condattr_init(&attr);
        pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&cond, &attr);
        pthread_condattr_destroy(&attr);
    } else{
        pthread_cond_init(&cond, NULL);
    }
}


inline int Condition::wait(Mutex& mutex) {
    return -pthread_cond_wait(&cond, &mutex.mutex);
}

inline int Condition::waitRelative(Mutex &mutex, int64_t time) {
    struct timeval tv;
    struct timespec ts;
    //获取当前时间
    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec;                  //秒
    ts.tv_nsec = tv.tv_usec * 1000;         //纳秒

    ts.tv_sec += time / 1000000000;
    ts.tv_nsec += time % 1000000000;

    if(ts.tv_nsec >= 1000000000){
        ts.tv_nsec -= 1000000000;
        ts.tv_sec++;
    }
    return -pthread_cond_timedwait(&cond, &mutex.mutex, &ts);
}


inline void Condition::signal() {
    pthread_cond_signal(&cond);
}

inline void Condition::broadcast() {
    pthread_cond_broadcast(&cond);
}



#endif //MYPLAYER_CONDITION_H
