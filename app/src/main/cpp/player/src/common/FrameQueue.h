//
// Created by jackzhous on 2019/8/16.
/**
 * 此队列比较复杂，需求如下：
 * 1. 队列基本功能，一边进，一边出，队列有最大现在max
 * 2. 队列元素为自定义结构体JFrame，包含了图像的解码帧数据
 * 3. 使用完后的不需要的图像帧要unref释放掉，记住是不需要的
 * 4. 图像刷新线程，需要参考上一帧、当前帧以及下一帧数据，所以不要盲目unref图像数据
 */
//FrameQueue 存放解码后的视频帧
#ifndef MYPLAYER_FRAMEQUEUE_H
#define MYPLAYER_FRAMEQUEUE_H

#include <Mutex.h>
#include "Config.h"
#include "Thread.h"

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
};

#define MAX 10

/**
 * 封装的数据
 */
typedef struct {
    AVFrame* frame;
    int frameWidth;
    int frameHeight;
    double pts;
    double duration;
//    AVSubtitle sub;
    int format;
    int upload;
}JFrame;

class FrameQueue{
public:
    FrameQueue(int max);
    ~FrameQueue();
    void flush();
    int getFrameLen() const ;
    void setAbort(short abort);
    JFrame* getpushFrame();            //返回地址让外部填入
    JFrame* getPopFrame();             //获取弹出地址
    void push();                    //pushFrame后调用该方法让栈加1
    void pop();                     //出栈，这里为什么要这么设计？ 1. 考虑AVFrame是一副二维图形，故直接地址引用效率比较高 2. 直接返回地址给外部使用，确认外部使用完后才能往这个地址继续写入内容，否则还没使用完就覆盖会出错
    JFrame* getLastFrame();
    JFrame* getCurrentFrame();
    JFrame* getNextFrame();
    void unrefFrame(JFrame* frame);     //释放frame
    void start();
    void stop();
    void startRender();
    void finishRender();

private:
    int queueMax;                   //总容量
    int inputIndex,outputIndex;     //进出序号
    int size;                       //当前size
    JFrame frame[MAX];
    short abort;
    Mutex mMutex;
    Condition mCond;
    bool exit;
    int finishFlag = 0;
};

#endif //MYPLAYER_FRAMEQUEUE_H
