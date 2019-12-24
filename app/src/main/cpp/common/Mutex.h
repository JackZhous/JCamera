//
// Created by jackzhous on 2019/8/9.
//

#ifndef MYPLAYER_MUTEX_H
#define MYPLAYER_MUTEX_H

#include <pthread.h>

class Condition;

class Mutex{
    public:
        enum{
            PRIVATE = 0,
            SHARED = 1
        };
        Mutex();
        Mutex(const char* name);
        Mutex(int type, const char* name);
        virtual ~Mutex();

        int lock();
        int unlock();
        int tryLock();

        class AutoLock{
            public:
                inline AutoLock(Mutex& lock) : mLock(lock) {mLock.lock();}
                inline AutoLock(Mutex* lock) : mLock(*lock) {mLock.lock();}
                inline ~AutoLock(){mLock.unlock();}
            private:
                Mutex& mLock;
        };

    private:
        pthread_mutex_t mutex;
        friend class Condition;

        //进制拷贝、复制
        Mutex(const Mutex&);
        Mutex& operator = (Mutex &);
};

inline Mutex::Mutex() {
    pthread_mutex_init(&mutex, NULL);
}

inline Mutex::Mutex(const char* name) {
    pthread_mutex_init(&mutex, NULL);
}

inline Mutex::Mutex(int type, const char* name){
    //设置共享线程
    if(type == SHARED){
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    } else{
        pthread_mutex_init(&mutex, NULL);
    }
}

inline Mutex::~Mutex() {
    pthread_mutex_destroy(&mutex);
}

inline int Mutex::lock() {
    return -pthread_mutex_lock(&mutex);
}

inline int Mutex::unlock() {
    return -pthread_mutex_unlock(&mutex);
}

inline int Mutex::tryLock() {
    return -pthread_mutex_trylock(&mutex);
}

typedef Mutex::AutoLock AutoMutex;

#endif //MYPLAYER_MUTEX_H
