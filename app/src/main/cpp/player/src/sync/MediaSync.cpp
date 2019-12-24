//
// Created by jackzhous on 2019/8/26.
//

#include <common/FrameQueue.h>
#include "MediaSync.h"


MediaSync::MediaSync(PlayerStatus* status) {
    this->playerStatus = status;
    abort = false;
    exit = true;
    audioClock = new MediaClock();
    videoClock = new MediaClock();
    extClock = new MediaClock();
    aDecoder = NULL;
    vDecoder = NULL;
    syncThread = NULL;

    forceFresh = 0;
    maxFrameDuration = 10.0;
    frameTimer = 0;
    frameTimerRefresh = 1;

    videoDevice = NULL;
    swsContext = NULL;
    mBuffer = NULL;
    pFrameARGB = NULL;
}


MediaSync::~MediaSync() {
    if(audioClock){
        delete audioClock;
        audioClock = NULL;
    }

    if(videoClock){
        delete videoClock;
        videoClock = NULL;
    }

    if(extClock){
        delete extClock;
        extClock = NULL;
    }
}

/**
 * 音视频同步
 */
void MediaSync::run() {
    double remaining_time = 0.0;
    while (true){
        if(abort || playerStatus->abortRequest){
            if(videoDevice != NULL){
                videoDevice->terminate(true);
            }
            break;
        }
        if(remaining_time > 0.0){
            av_usleep((int64_t)(remaining_time * 1000000.0));
        }

        remaining_time = REFRESH_RATE;
        if(!playerStatus->pauseRequest || forceFresh){
            refreshVideo(&remaining_time);
        }
    }

    exit = true;
    mCond.signal();
}

void MediaSync::refreshVideo(double *remain_time) {
    double time;

    //检查外部时钟
    if(!playerStatus->pauseRequest && playerStatus->syncType == AV_SYNC_EXTERNAL){
        checkExternalClockSpeed();
    }
    for(;;){
        vDecoder->getFrameQueue()->startRender();
        if(playerStatus->abortRequest || !vDecoder){
            break;
        }

        //帧队列是否存在数据
        if(vDecoder->getFrameSize() > 0){
            double lastDuration, duration, delay;
            JFrame *currentFrame, *lastFrame;

            lastFrame = vDecoder->getFrameQueue()->getLastFrame();
            currentFrame = vDecoder->getFrameQueue()->getCurrentFrame();
            // 判断是否需要强制更新帧的时间,定位视频触发
            if(frameTimerRefresh){
                frameTimer = av_gettime_relative() / 1000000.0;
                frameTimerRefresh = 0;
            }

            //暂停状态直接显示
            if(playerStatus->abortRequest || playerStatus->pauseRequest){
                break;
            }

            //计算上一次显示的时长
            lastDuration = calculateDuration(lastFrame, currentFrame);
            //根据上一次显示时长来计算时延
            delay = calculateDelay(lastDuration);
            if(fabs(delay) > AV_SYNC_THRESHOLD_MAX){
                if(delay > 0){
                    delay = AV_SYNC_THRESHOLD_MAX;
                } else{
                    delay = 0;
                }
            }

            time = av_gettime_relative() / 1000000.0;
            if(isnan(frameTimer) || time < frameTimer){
                frameTimer = time;
            }

            if(time < frameTimer + delay){
                *remain_time = FFMIN(frameTimer + delay - time, *remain_time);
                break;
            }

            //更新帧计时器
            frameTimer += delay;
            // 帧计时器落后当前时间超过了阈值，则用当前的时间作为帧计时器时间
            if(delay > 0 && time - frameTimer > AV_SYNC_THRESHOLD_MAX){
                frameTimer = time;
            }


            //更新视频时钟
            mMutex.lock();
            if(!isnan(currentFrame->pts)){
                videoClock->setClock(currentFrame->pts);
                extClock->syncToSlave(videoClock);
            }
            mMutex.unlock();

            // 如果队列中还剩余超过一帧的数据时，需要拿到下一帧，然后计算间隔，并判断是否需要进行舍帧操作
            if(vDecoder->getFrameSize() > 1){
                JFrame *nextFrame = vDecoder->getFrameQueue()->getNextFrame();
                duration = calculateDuration(currentFrame, nextFrame);
                //如果系统时钟比下一帧播放时间都还大，就没必要显示了
                if(time > frameTimer + duration){
                    vDecoder->getFrameQueue()->finishRender();
                    vDecoder->getFrameQueue()->pop();
                    continue;
                }
            }
            //下一帧
            vDecoder->getFrameQueue()->finishRender();
            vDecoder->getFrameQueue()->pop();
            forceFresh = 1;
        }

        break;
    }

    //刷新画面
    if(forceFresh == 1){
        renderVideo();
    }

    forceFresh = 0;
}

//刷新画面
void MediaSync::renderVideo() {

    if(!videoClock || !videoDevice){
        return;
    }
    mMutex.lock();
        //虽然是last，但是上一个函数pop一次，就是当前帧
        JFrame* vp = vDecoder->getFrameQueue()->getLastFrame();
        int ret = 0;
        if(!vp->upload){
            vp->upload = 1;
            switch (vp->frame->format){
                // YUV420P 和 YUVJ420P 除了色彩空间不一样之外，其他的没什么区别
                // YUV420P表示的范围是 16 ~ 235，而YUVJ420P表示的范围是0 ~ 255
                // 这里做了兼容处理，后续可以优化，shader已经过验证

                case AV_PIX_FMT_YUV420P:
                case AV_PIX_FMT_YUVJ420P:
                    videoDevice->onInitTexture(vp->frame->width, vp->frame->height, FMT_YUV420P, BLEND_NONE, vDecoder->getRorate());
                    //linesize即平面长度 YUV三个通过平面存储
                    if(vp->frame->linesize[0] < 0 || vp->frame->linesize[1] < 0 || vp->frame->linesize[2] < 0){
                        LOGE("negative linesize is not supported for YUV");
                        break;
                    }
                    ret = videoDevice->onUpdateYUV(vp->frame->data[0], vp->frame->linesize[0],
                                                    vp->frame->data[1], vp->frame->linesize[1],
                                                    vp->frame->data[2], vp->frame->linesize[2]);
                    if(ret < 0){
                        LOGE("update yuv failed");
                        break;
                    }
                    break;

                //这是BGRA格式，在底层glsl中会进行颜色分量转换
                case AV_PIX_FMT_BGRA:
                    videoDevice->onInitTexture(vp->frame->width, vp->frame->height, FMT_ARGB, BLEND_NONE, 90);
                    ret = videoDevice->onUpdateARGB(vp->frame->data[0], vp->frame->linesize[0]);
                    if(ret < 0){
                        LOGE("update rgba failed");
                        break;
                    }
                    break;

                case -1:
                    break;

                //其他格式转码成RGBA渲染
                default:
                    swsContext = sws_getCachedContext(swsContext, vp->frame->width, vp->frame->height,
                                                      (AVPixelFormat)vp->frame->format,
                                                       vp->frame->width, vp->frame->height,
                                                        AV_PIX_FMT_BGRA, SWS_BICUBIC, NULL, NULL, NULL);
                    if(!mBuffer){
                        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, vp->frame->width,
                                                            vp->frame->height, 1);
                        mBuffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
                        pFrameARGB = av_frame_alloc();
                        av_image_fill_arrays(pFrameARGB->data, pFrameARGB->linesize, mBuffer,
                                            AV_PIX_FMT_BGRA, vp->frame->width, vp->frame->height, 1);
                    }
                    if(swsContext){
                        sws_scale(swsContext, (uint8_t const * const *)vp->frame->data, vp->frame->linesize, 0, vp->frame->height,
                                pFrameARGB->data, pFrameARGB->linesize);
                    }

                    videoDevice->onInitTexture(vp->frame->width, vp->frame->height, FMT_ARGB, BLEND_NONE, vDecoder->getRorate());
                    ret = videoDevice->onUpdateARGB(pFrameARGB->data[0], pFrameARGB->linesize[0]);
                    if(ret < 0){
                        LOGE("update sws rgba failed");
                        mMutex.unlock();
                        return;
                    }
                    break;
            }
        }
    if(videoDevice){
        videoDevice->onRequestRender(vp->frame->linesize[0] < 0);
    }
    mMutex.unlock();
}


double MediaSync::calculateDuration(JFrame *vp, JFrame *nextvp) {
    double duration = nextvp->pts - vp->pts;
    if(isnan(duration) || duration <= 0 || duration > maxFrameDuration){
        return vp->duration;
    } else{
        return duration;
    }
}

double MediaSync::calculateDelay(double delay) {
    double syncThreshold, diff = 0;
    if(playerStatus->syncType != AV_SYNC_VIDEO){
        diff = videoClock->getClock() - getMasterClock();       //计算两个时钟的差值
        //约定delay的值不超过MIN  MAX之间
        syncThreshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(delay, AV_SYNC_THRESHOLD_MAX));
        if(!isnan(diff) && fabs(diff) < maxFrameDuration){
            //视频时钟小于主时钟，要减小时延
            if(diff < -syncThreshold){
                delay = FFMAX(0, delay+diff);
            //视频时钟大大超过主时钟,增大延时
            } else if(diff >= syncThreshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD){
                delay = delay + diff;
            //视频时钟超前，增大时延即可
            } else if(diff >= syncThreshold){
                delay = 2 * delay;
            }
        }
    }

    return delay;
}

void MediaSync::checkExternalClockSpeed() {

}

void MediaSync::updateExternalClock(double pts) {}

void MediaSync::stop() {
    mMutex.lock();
    abort = true;
    mCond.signal();
    mMutex.unlock();



    //为什么要分开写，是因为run中结束后又ext标记
    mMutex.lock();
    while (!exit){
        mCond.wait(mMutex);
    }
    mMutex.unlock();

    if(syncThread == NULL){
        return;
    }
    syncThread->join();
    delete syncThread;
    syncThread = NULL;
    LOGI("同步线程结束");
}

void MediaSync::setVideoDevice(VideoDevice *device) {
    videoDevice = device;
}

void MediaSync::setMaxFrameDuration(double time) {
    maxFrameDuration = time;
}


void MediaSync::updateAudioColock(double pts, double time) {
    audioClock->setClock(pts, time);
    extClock->syncToSlave(audioClock);
}

double MediaSync::getAudioDffClock() {
    return audioClock->getClock() - getMasterClock();
}

double MediaSync::getMasterClock() {
    double val = 0;
    switch (playerStatus->syncType){
        case AV_SYNC_VIDEO:
            val = videoClock->getClock();
            break;

        case AV_SYNC_AUDIO:
            val = audioClock->getClock();
            break;

        case AV_SYNC_EXTERNAL:
            val = extClock->getClock();
            break;
    }

    return val;
}


MediaClock* MediaSync::getAudioClock() {
    return audioClock;
}


MediaClock* MediaSync::getVideoClock() {
    return videoClock;
}

MediaClock* MediaSync::getExtClock() {
    return extClock;
}

void MediaSync::refreshVideoTImer() {
    mMutex.lock();
    frameTimerRefresh = 1;
    mCond.signal();
    mMutex.unlock();
}


void MediaSync::start(AudioDecoder *audioDecoder, VideoDecoder *videoDecoder) {
    mMutex.lock();
    this->aDecoder = audioDecoder;
    this->vDecoder = videoDecoder;
    abort = false;
    exit = false;
    mCond.signal();
    mMutex.unlock();
    if(videoDevice && !syncThread){
        syncThread = new Thread(this);
        syncThread->start();
    }
}

void MediaSync::reset() {
    stop();
    playerStatus = NULL;
    vDecoder = NULL;
    aDecoder = NULL;
    videoDevice = NULL;

    if(pFrameARGB){
        av_frame_free(&pFrameARGB);
        av_free(pFrameARGB);
        pFrameARGB = NULL;
    }

    if(mBuffer){
        av_freep(&mBuffer);
        mBuffer = NULL;
    }

    if(swsContext){
        sws_freeContext(swsContext);
        swsContext = NULL;
    }
}