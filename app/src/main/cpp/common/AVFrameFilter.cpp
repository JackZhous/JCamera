//
// Created by jackzhous on 2019/12/27.
//

#include <AndroidLog.h>

#include "AVFrameFilter.h"

#define RATIONAL_MAX 1000000
#define SAMPLE_SIZE 1024

AVFrameFilter::AVFrameFilter() {
    av_register_all();
    avfilter_register_all();

    mWidth = 0;
    mHeight = 0;
    mFrameRate = 0;
    mInputPixelFormat = AV_PIX_FMT_NONE;
    mOutPixelFormat = AV_PIX_FMT_NONE;
    mVideoFilter = "null";
    mVideoEnable = false;
    mVideoBuffersrcCtx = nullptr;
    mVideoBuffersinkCtx = nullptr;
    mVideoFilterGraph = nullptr;

    mInSampleFormat = AV_SAMPLE_FMT_NONE;
    mOutSampleFormat = AV_SAMPLE_FMT_NONE;
    mInSampleRate = 0;
    mInChannels = 0;
    mOutChannels = 0;
    mAudioFilter = "anull";
    mAudioBuffersinkCtx = nullptr;
    mAudioBuffersrcCtx = nullptr;
    mAudioFilterGraph = nullptr;
    mAudioEnable = false;
}

AVFrameFilter::~AVFrameFilter() {
    release();
}


void AVFrameFilter::release() {
    mVideoBuffersinkCtx = nullptr;
    mVideoBuffersrcCtx = nullptr;
    if(mVideoFilterGraph != nullptr){
        avfilter_graph_free(&mVideoFilterGraph);
        mVideoFilterGraph = nullptr;
    }

    mAudioBuffersrcCtx = nullptr;
    mAudioBuffersinkCtx = nullptr;
    if(mAudioFilterGraph != nullptr){
        avfilter_graph_free(&mAudioFilterGraph);
        mAudioFilterGraph = nullptr;
    }
}

void AVFrameFilter::setVideoInput(int width, int height, AVPixelFormat pixelFormat, int frameRate,
                                  const char *filter) {
    mWidth = width;
    mHeight = height;
    mInputPixelFormat = pixelFormat;
    mFrameRate = frameRate;
    if(mOutPixelFormat == AV_PIX_FMT_NONE){
        mOutPixelFormat = pixelFormat;
    }
    mVideoFilter = (filter == nullptr) ? "null": filter;
    mVideoEnable = (strcmp(mVideoFilter, "null") != 0);;
}


void AVFrameFilter::setVideoOutput(AVPixelFormat format) {
    mOutPixelFormat = format;
}

void AVFrameFilter::setAudioInput(int sampleRate, int channels, AVSampleFormat format,
                                  const char *filter) {
    mInSampleRate = sampleRate;
    mInChannels = channels;
    mInSampleFormat = format;
    if(mOutSampleFormat == AV_SAMPLE_FMT_NONE){
        mOutSampleFormat = format;
    }

    if(mOutSampleRate == 0){
        mOutSampleRate = sampleRate;
    }
    if(mOutChannels == 0){
        mOutChannels = channels;
    }

    mAudioFilter = (filter == nullptr) ? "anull" : filter;

    mAudioEnable = (strcmp(mAudioFilter, "anull") != 0);
}

int AVFrameFilter::initFilter() {
    if(mVideoEnable){
        initVIdeoFilter();
    }

    if(mAudioEnable){
        initAudioFilter();
    }

    return 0;
}


int AVFrameFilter::initAudioFilter() {
    int ret = 0;
    AVRational timebase = av_inv_q(av_d2q(mInSampleRate, RATIONAL_MAX));

    char args[512];
    AVFilter *bufferSrc = nullptr;
    AVFilter *bufferSink = nullptr;
    //avfilter_inout_alloc分配一个AVFilterInOut的结构体
    AVFilterInOut *output = avfilter_inout_alloc();
    AVFilterInOut *in = avfilter_inout_alloc();
    //avfilter_graph_alloc分配一个AVFilterGraph结构体
    mAudioFilterGraph = avfilter_graph_alloc();
    int64_t outChannelLayout = av_get_default_channel_layout(mOutChannels);
    if(!output || !in || !mAudioFilterGraph){
        LOGE("failed to allocte audio filter graph object");
        goto end;
    }

    bufferSink = avfilter_get_by_name("abuffersink");
    bufferSrc = avfilter_get_by_name("abuffer");
    if(!bufferSink || !bufferSrc){
        LOGE("Failed to found source or sink element");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    // 设置音频过滤链参数
    snprintf(args, sizeof(args),
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=%lld",
             timebase.num, timebase.den, mInSampleRate, av_get_sample_fmt_name(mInSampleFormat),
             av_get_default_channel_layout(mInChannels));

    LOGI("audio filter: %s", args);
    // 创建音频过滤器输入缓冲区
    ret = avfilter_graph_create_filter(&mAudioBuffersrcCtx, bufferSrc, "in",
                                       args, nullptr, mAudioFilterGraph);
    if (ret < 0) {
        LOGE("Failed to create audio buffer source");
        goto end;
    }

    // 创建音频过滤器输出缓冲区
    ret = avfilter_graph_create_filter(&mAudioBuffersinkCtx, bufferSink, "out",
                                       nullptr, nullptr, mAudioFilterGraph);
    if (ret < 0) {
        LOGE("Failed to create audio buffer sink");
        goto end;
    }

    // 设置输出采样率格式
    if (mOutSampleFormat != AV_SAMPLE_FMT_NONE) {
        ret = av_opt_set_bin(mAudioBuffersinkCtx, "sample_fmts", (uint8_t *) &mOutSampleFormat,
                             sizeof(mOutSampleFormat),
                             AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            LOGE("Failed to set output sample format");
            goto end;
        }
    }

    // 设置输出声道格式
    ret = av_opt_set_bin(mAudioBuffersinkCtx, "channel_layouts", (uint8_t *) &outChannelLayout,
                         sizeof(outChannelLayout), AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOGE("Failed to set output channel layout");
        goto end;
    }

    // 设置输出采样率
    ret = av_opt_set_bin(mAudioBuffersinkCtx, "sample_rates", (uint8_t *) &mOutSampleRate,
                         sizeof(mOutSampleRate), AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOGE("Failed to set output sample rate");
        goto end;
    }

    // 绑定输入端
    output->name = av_strdup("in");
    output->filter_ctx = mAudioBuffersrcCtx;
    output->pad_idx = 0;
    output->next = nullptr;

    // 绑定输出端
    in->name = av_strdup("out");
    in->filter_ctx = mAudioBuffersinkCtx;
    in->pad_idx = 0;
    in->next = nullptr;

    if (!output->name || !in->name) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    // 解析Filter描述
    if ((ret = avfilter_graph_parse_ptr(mAudioFilterGraph, mAudioFilter,
                                        &in, &output, nullptr)) < 0) {
        LOGE("Failed to call avfilter_graph_parse_ptr: %s", av_err2str(ret));
        goto end;
    }

    if ((ret = avfilter_graph_config(mAudioFilterGraph, nullptr)) < 0) {
        LOGE("Failed to call avfilter_graph_config: %s", av_err2str(ret));
        goto end;
    }

    end:
    avfilter_inout_free(&in);
    avfilter_inout_free(&output);

    return ret;
}

/**
 * 初始化视频AVFilter
 */
int AVFrameFilter::initVIdeoFilter() {
    int ret = 0;
    AVRational timeBase = av_inv_q(av_d2q(mFrameRate, RATIONAL_MAX));
    AVRational ratio = av_d2q(1, 255);

    char args[512];
    AVFilter *bufferSrc = nullptr;
    AVFilter *bufferSink = nullptr;
    //avfilter_inout_alloc分配一个AVFilterInOut的结构体
    AVFilterInOut *output = avfilter_inout_alloc();
    AVFilterInOut *in = avfilter_inout_alloc();
    //avfilter_graph_alloc分配一个AVFilterGraph结构体
    mVideoFilterGraph = avfilter_graph_alloc();
    if(!output || !in || !mVideoFilterGraph){
        LOGE("failed to allocte video filter graph object");
        goto end;
    }
    //根据名字去查找过滤器
    bufferSrc = avfilter_get_by_name("buffer");
    bufferSink = avfilter_get_by_name("buffersink");
    if(!bufferSink || !bufferSrc){
        LOGE("failed to found filtering source or sink element");
        goto end;
    }

    //设置视频过滤参数
    snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                                mWidth, mHeight, mInputPixelFormat, timeBase.num, timeBase.den,
                                ratio.num, ratio.den);
    LOGI("video filter: %s", args);
    //创建视频过滤器输入缓冲区 并且初始化
    ret = avfilter_graph_create_filter(&mVideoBuffersrcCtx, bufferSrc, "in", args, nullptr,
                                            mVideoFilterGraph);
    if(ret < 0){
        LOGE("failed to create video buffer source");
        goto end;
    }

    //创建视频过滤输出缓冲区
    ret = avfilter_graph_create_filter(&mVideoBuffersinkCtx, bufferSink, "out", nullptr, nullptr,
                                            mVideoFilterGraph);

    if(ret < 0){
        LOGE("failed to create video buffer sink");
        goto end;
    }

    //设置像素转换过滤参数
    ret = av_opt_set_bin(mVideoBuffersinkCtx, "pix_fmts", (uint8_t*)&mOutPixelFormat,
            sizeof(mOutPixelFormat), AV_OPT_SEARCH_CHILDREN);
    if(ret < 0){
        LOGE("failed to set output pixel format");
        goto end;
    }

    //绑定输入端
    output->name = av_strdup("in");
    output->filter_ctx = mVideoBuffersrcCtx;
    //filter_ctx的序号
    output->pad_idx = 0;
    output->next = nullptr;

    //绑定输出端
    in->name = av_strdup("out");
    in->filter_ctx = mVideoBuffersinkCtx;
    in->pad_idx = 0;
    in->next = nullptr;

    if(!output->name || !in->name){
        ret = AVERROR(ENOMEM);
        goto end;
    }

    //解析filter描述
    if((ret = avfilter_graph_parse_ptr(mVideoFilterGraph, mVideoFilter, &in, &output, nullptr)) < 0){
        LOGE("failed to call avfilter_graph_parse_ptr: %s", av_err2str(ret));
        goto end;
    }

    //解析filter描述
    if((ret = avfilter_graph_config(mVideoFilterGraph, nullptr)) < 0){
        LOGE("failed to call avfilter_graph_config: %s", av_err2str(ret));
        goto end;
    }

    end:
    avfilter_inout_free(&output);
    avfilter_inout_free(&in);

    return ret;
}


int AVFrameFilter::filterData(AVMediaData* data) {
    if(data->type == MediaAudio){
        return filterAudio(data);
    } else if (data->type == MediaVideo){
        return filterVideo(data);
    }
    LOGE("unknown data type");
    return -1;
}

int AVFrameFilter::filterAudio(AVMediaData *data) {
    if(!mAudioEnable){
        LOGE("unable audio filter");
        return 0;
    }

    int ret;
    AVFrame *srcFrame = av_frame_alloc();
    if(!srcFrame){
        LOGE("failed to allocate src frame");
        return -1;
    }

    //将音频数据复制到输入帧
    ret = av_samples_fill_arrays(srcFrame->data, srcFrame->linesize, data->sample, mInChannels,
            SAMPLE_SIZE, mInSampleFormat, 1);

    if(ret < 0){
        LOGE("failed to call av_samples_fill_arrays: %s ", av_err2str(ret));
        freeFrame(srcFrame);
        return ret;
    }

    srcFrame->sample_rate = mInSampleFormat;
    srcFrame->channel_layout = (uint64_t)av_get_default_channel_layout(mInChannels);
    srcFrame->channels = mInChannels;
    srcFrame->format = mInSampleFormat;
    srcFrame->nb_samples = SAMPLE_SIZE;

    //将输入帧放入过滤器输入端
    ret = av_buffersrc_add_frame_flags(mAudioBuffersrcCtx, srcFrame, 0);
    if(ret < 0){
        LOGE("audio failed to call av_buffersrc_add_frame_flags: %s", av_err2str(ret));
        freeFrame(srcFrame);
        return ret;
    }

    //创建输出端
    AVFrame *dstFrame = av_frame_alloc();
    if(!dstFrame){
        LOGE("failed to allocate dst frame");
        freeFrame(dstFrame);
        return -1;
    }

    //从filter输出端取出音频帧
    ret = av_buffersink_get_frame(mAudioBuffersinkCtx, dstFrame);
    if(ret < 0){
        LOGE("failed to call av_buffersink_get_frame : %s", av_err2str(ret));
        freeFrame(srcFrame);
        freeFrame(dstFrame);
        return -1;
    }
    //计算filter后的音频帧大小
    int size = av_samples_get_buffer_size(dstFrame->linesize, dstFrame->channels,
            dstFrame->nb_samples, AVSampleFormat(dstFrame->format), 1);

    if(size < 0){
        LOGE("failed to call av_samples_get_buffer_size :%s", av_err2str(size));
        freeFrame(srcFrame);
        freeFrame(dstFrame);
        return -1;
    }

    //创建缓冲区
    uint8_t *sampleBuffer = (uint8_t*)av_malloc((size_t)size);
    if(sampleBuffer == nullptr){
        LOGE("failed to allocate memory");
        freeFrame(srcFrame);
        freeFrame(dstFrame);
        return -1;
    }

    data->free();
    data->sample = sampleBuffer;
    data->sample_size = size;

    freeFrame(srcFrame);
    freeFrame(dstFrame);
    return 0;
}

int AVFrameFilter::filterVideo(AVMediaData *data) {
    if(!mVideoEnable){
        LOGE("can not filter video");
        return 0;
    }

    int ret;
    //创建输入帧
    AVFrame* srcFrame = av_frame_alloc();
    if(!srcFrame){
        LOGE("failed to allcate src frame");
        return -1;
    }

    ret = av_image_fill_arrays(srcFrame->data, srcFrame->linesize, data->image, mInputPixelFormat,
            mWidth, mHeight, 1);
    if(ret < 0){
        LOGE("failed to call av_image_fill_arrays : %s", av_err2str(ret));
        freeFrame(srcFrame);
        return ret;
    }

    srcFrame->width = mWidth;
    srcFrame->height = mHeight;
    srcFrame->format = mInputPixelFormat;

    //将输入放入filter输入端
    ret = av_buffersrc_add_frame_flags(mVideoBuffersrcCtx, srcFrame, 0);
    if(ret < 0){
        LOGE("video failed to call av_buffersrc_add_frame_flags: %s", av_err2str(ret));
        freeFrame(srcFrame);
        return ret;
    }

    AVFrame* dstFrame = av_frame_alloc();
    if(!dstFrame){
        LOGE("failed to allocate dst frame");
        freeFrame(srcFrame);
        return -1;
    }

    ret = av_buffersink_get_frame(mVideoBuffersinkCtx, dstFrame);
    if(ret < 0){
        LOGE("failed to call av_buffersink_get_frame: %s", av_err2str(ret));
        freeFrame(srcFrame);
        freeFrame(dstFrame);
        return ret;
    }

    //计算过滤后的大小
    int size = av_image_get_buffer_size(AVPixelFormat(dstFrame->format), dstFrame->width, dstFrame->height, 1);
    if(size < 0){
        LOGE("failed to get image buffer size: %s", av_err2str(size));
        freeFrame(srcFrame);
        freeFrame(dstFrame);
        return -1;
    }

    //创建缓冲区
    uint8_t * imageBuffer = (uint8_t*)av_malloc((size_t)size);
    if(imageBuffer == nullptr){
        LOGE("failed allocate image buffer");
        freeFrame(srcFrame);
        freeFrame(dstFrame);
        return -1;
    }

    ret = av_image_copy_to_buffer(imageBuffer, size, (const uint8_t**)dstFrame->data, dstFrame->linesize,
            AVPixelFormat(dstFrame->format), dstFrame->width, dstFrame->height, 1);
    if(ret < 0){
        freeFrame(srcFrame);
        freeFrame(dstFrame);
        av_free(imageBuffer);
        return -1;
    }

    data->free();
    data->image = imageBuffer;
    data->length = size;
    data->width = dstFrame->width;
    data->height = dstFrame->height;
    data->pixelFormat = pixelFormatConvert(AVPixelFormat(dstFrame->format));

    freeFrame(srcFrame);
    freeFrame(dstFrame);

    return 0;
}


void AVFrameFilter::freeFrame(AVFrame *frame) {
    if(frame){
        av_frame_free(&frame);
    }
}
