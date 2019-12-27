//
// Created by jackzhous on 2019/12/27.
//

#ifndef JCAMERA_SAFETYQUEUE_H
#define JCAMERA_SAFETYQUEUE_H

#include <mutex>
#include <queue>

template <typename  T>
class SafetyQueue {
public:
    SafetyQueue(){}

    void push(T elelment){
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(elelment);
    }

    T pop(){
        std::lock_guard<std::mutex> lock(mutex);
        T ret = queue.front();
        queue.pop();
        return ret;
    }

    bool empty(){
        std::lock_guard<std::mutex> lock(mutex);
        return  queue.empty();
    }

    int size(){
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }


private:
    mutable std::mutex mutex;
    std::queue<T> queue;
};


#endif //JCAMERA_SAFETYQUEUE_H
