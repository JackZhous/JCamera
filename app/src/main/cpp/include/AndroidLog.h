//
// Created by jackzhous on 2019/8/8.
//

#ifndef MYPLAYER_ANDROIDLOG_H
#define MYPLAYER_ANDROIDLOG_H

#include <stdint.h>
#include <time.h>
#include <android/log.h>
#define TAG "j_tag"

/**
 * __ANDROID__是在Android系统中定义的，就像Windows的WIN32一样
 */
#ifdef __ANDROID__

/**
 * ##__VA_ARGS__代表一个可变参数，签名的##表示如果可变参数为空，会自动去掉前面的逗号
 * 宏里面不可以使用...省略号
 */
#define LOGI(format, ...) __android_log_print(ANDROID_LOG_INFO, TAG, format, ##__VA_ARGS__)
#define LOGE(format, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, format, ##__VA_ARGS__)
#define LOGV(format, ...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, format, ##__VA_ARGS__)
#define LOGW(format, ...) __android_log_print(ANDROID_LOG_WARN, TAG, format, ##__VA_ARGS__)

#else

#define LOGI(format, ...) {}
#define LOGE(format, ...) {}
#define LOGV(format, ...) {}
#define LOGW(format, ...) {}
#endif


// 获取当前时钟(ms)
inline uint64_t getCurrentTimeMs() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    uint64_t us = (uint64_t) (tv.tv_sec) * 1000 * 1000 + (uint64_t) (tv.tv_usec);
    return us / 1000;
}

#endif //MYPLAYER_ANDROIDLOG_H