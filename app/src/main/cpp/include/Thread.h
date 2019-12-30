//
// Created by jackzhous on 2019/8/9.
//

#ifndef MYPLAYER_THREAD_H
#define MYPLAYER_THREAD_H

#include "Condition.h"

typedef enum {
    Priority_Default = -1,
    Priority_Low = 0,
    Priority_Normal = 1,
    Priority_High = 2
} ThreadPriority;

class Runnable{
    public:
        virtual ~Runnable(){}
        virtual void run() = 0;
};

class Thread : public Runnable{
    public:
        Thread();
        Thread(ThreadPriority priority);
        Thread(Runnable *runnable);
        Thread(Runnable *runnable, ThreadPriority priority);
        ~Thread();

        void start();
        void join();
        void detach();
        pthread_t getId() const ;   //此方法只能读取，不能写
        bool isActive() const ;

    protected:
        virtual void run();
        int setPriority(ThreadPriority priority);
        //子线程入口函数
        static void *threadEntry(void *arg);

    protected:
        Mutex mutex;
        Condition condition;
        Runnable *mRunnable;
        pthread_t mid;
        bool mNeedJoin;             //对应java中的join方法一样的意思
        bool mRunning;
        ThreadPriority mPriority;
};


inline Thread::Thread() {
    mid = -1;
    mNeedJoin = true;
    mRunning = false;
    mPriority = Priority_Default;
    mRunnable = NULL;
}

inline Thread::Thread(ThreadPriority priority) {
    mPriority = priority;
    mid = -1;
    mNeedJoin = true;
    mRunning = false;
    mRunnable = NULL;
}

inline Thread::Thread(Runnable *runnable) {
    mPriority = Priority_Default;
    mid = -1;
    mNeedJoin = false;
    mRunning = false;
    mRunnable = runnable;
}

inline Thread::Thread(Runnable *runnable, ThreadPriority priority) {
    mPriority = priority;
    mid = -1;
    mNeedJoin = false;
    mRunning = false;
    mRunnable = runnable;
}

inline void Thread::run() {}

/**
 * 设置线程优先级
 */
inline int Thread::setPriority(ThreadPriority priority) {
    if(priority == Priority_Default){
        return 0;
    }

    struct sched_param sched;
    int policy;
    pthread_t thread = pthread_self();

    if(pthread_getschedparam(thread, &policy, &sched) < 0){
        return -1;
    }

    if(priority == Priority_Low ){
        sched.sched_priority = sched_get_priority_min(policy);
    }else if(priority == Priority_High){
        sched.sched_priority = sched_get_priority_max(policy);
    } else {
        int min = sched_get_priority_min(policy);
        int max = sched_get_priority_max(policy);
        sched.sched_priority = min + (min + max) / 2;
    }

    if(pthread_setschedparam(thread, policy, &sched) < 0){
        return -1;
    }

    return 0;
}


inline void* Thread::threadEntry(void *arg) {
    Thread *thread = (Thread *)arg;

    if(thread != NULL){
        thread->mRunning = true;
        thread->condition.signal();
        thread->setPriority(thread->mPriority);

        if(thread->mRunnable != NULL){
            thread->mRunnable->run();
        } else{
            thread->run();
        }

        thread->mRunning = false;
        thread->condition.signal();
    }

    pthread_exit(NULL);
}


inline void Thread::start() {
    if(!mRunning){
        pthread_create(&mid, NULL, threadEntry, this);
        mNeedJoin = true;
    }

    mutex.lock();
    while (!mRunning){
        condition.wait(mutex);
    }
    mutex.unlock();
}

inline void Thread::join() {
    AutoMutex lock(mutex);
    if(mNeedJoin && mid > 0){
        pthread_join(mid, NULL);
        mNeedJoin = false;
        mid = -1;
    }
}

inline void Thread::detach() {
    AutoMutex lock(mutex);
    if(mid >= 0){
        pthread_detach(mid);
        mNeedJoin = false;
    }
}

inline pthread_t Thread::getId() const {
    return mid;
}

inline bool Thread::isActive() const {
    return mRunning;
}

inline Thread::~Thread() {
    join();
    mRunnable = NULL;
}


#endif //MYPLAYER_THREAD_H
