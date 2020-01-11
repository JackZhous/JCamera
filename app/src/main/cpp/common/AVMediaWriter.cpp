//
// Created by jackzhous on 2019/12/27.
//


#include "AVMediaWriter.h"

AVMediaWriter::~AVMediaWriter() {
    release();
}

AVMediaWriter::AVMediaWriter() {
    reset();
}

void AVMediaWriter::setOutoutPath(const char *dstUrl) {
    mDstUrl = dstUrl;
}

void AVMediaWriter::addEncodeOptions(std::string key, std::string value) {
    mEncodeOptions[key] = value;
}

void AVMediaWriter::setAudioEncoderName(const char *encoder) {
    mAudioEncodeName = av_strdup(encoder);
}

void AVMediaWriter::setVideoEncodeName(const char *coder) {
    mVideoEncodeName = av_strdup(coder);
}

void AVMediaWriter::setQuality(int quality) {
    //字符串转换流
    std::stringstream ss;
    ss << quality;
    std::string str;
    ss >> str;
    mEncodeOptions["crf"] = str;
}


void AVMediaWriter::setOutputVideo(int width, int height, int frameRate, AVPixelFormat pixFormat) {
    mWidth = width;
    mHeight = height;
    mPixelFormat = pixFormat;
    mFrameRate = frameRate;
    mVideoCodecId = AV_CODEC_ID_H264;
    mHasVideo = mWidth > 0 && mHeight > 0 && mPixelFormat != AV_PIX_FMT_NONE;
}

void AVMediaWriter::setOutputAudio(int sampleRate, int channel, AVSampleFormat sampleFormat) {
    mSampleRate = sampleRate;
    mChannels = channel;
    mSampleFormat = sampleFormat;
    mAudioCodecId = AV_CODEC_ID_AAC;
    mHasAudio = mSampleRate > 0 || mChannels > 0 && mSampleFormat != AV_SAMPLE_FMT_NONE;
}


int AVMediaWriter::prepare() {
    //去掉奇数宽度，为什么必须是偶数 因为YUV420的高宽必须为偶数，两行公用UV 一行取U 一行取V
    if(mWidth % 2 == 1){
        if(mHeight >= mWidth){
            mHeight = (int) (1.0 * (mWidth - 1) / mWidth * mHeight);
            mHeight = mHeight % 2 == 1 ? mHeight - 1 : mHeight;
        }
        mWidth--;
    }

    if(mHeight % 2 == 1){
        if(mWidth >= mHeight){
            mWidth = (int) (1.0 * (mHeight - 1) / mWidth * mHeight);
            mWidth = mWidth % 2 == 1 ? mWidth - 1 : mWidth;
        }
        mHeight--;
    }
    //打开输出文件
    return openOutputFile();
}

int AVMediaWriter::openOutputFile() {
    int ret;
    av_register_all();

    ret = avformat_alloc_output_context2(&pFormatCtx, nullptr, nullptr, mDstUrl);
    if(ret < 0 || !pFormatCtx){
        LOGE("failed to call avformat_alloc_output_context2: %s", av_err2str(ret));
        return AVERROR_UNKNOWN;
    }

    //打开视频编码器
    if(mHasVideo && (ret = openEncoder(AVMEDIA_TYPE_VIDEO) < 0)){
        LOGE("failed to Open video encoder context: %s", av_err2str(ret));
        return ret;
    }
    if(mHasAudio && (ret = openEncoder(AVMEDIA_TYPE_AUDIO) < 0)){
        LOGE("failed to Open audio encoder context: %s", av_err2str(ret));
        return ret;
    }

    //如果存在视频流，创建视频缓冲帧对象

    if(mHasVideo){
        mImageFrame = av_frame_alloc();
        if(!mImageFrame){
            LOGE("failed to allocate video frame");
            return -1;
        }
        mImageFrame->format = mPixelFormat;
        mImageFrame->width = mWidth;
        mImageFrame->height = mHeight;
        mImageFrame->pts = 0;

        int size = av_image_get_buffer_size(mPixelFormat, mWidth, mHeight, 1);
        if(size < 0){
            LOGE("failed to get image buffer size: %s", av_err2str(size));
            return -1;
        }

        mImageBuffer = (uint8_t *)av_malloc((size_t)size);
        if(!mImageBuffer){
            LOGE("failed to allocate image buffer");
            return -1;
        }
    }

    if(mHasAudio){
        mSampleFrame = av_frame_alloc();
        if(!mSampleFrame){
            LOGE("failed to allocate audio frame");
            return -1;
        }
        mSampleFrame->format = pAudioCodecCtx->sample_fmt;
        mSampleFrame->nb_samples = pAudioCodecCtx->frame_size;
        mSampleFrame->channel_layout = pAudioCodecCtx->channel_layout;
        mSampleFrame->pts = 0;

        //是否为多声道
        mSamplePlanes = av_sample_fmt_is_planar(pAudioCodecCtx->sample_fmt) ? pAudioCodecCtx->channels : 1;
        //缓冲区大小
        mSampleSize = av_samples_get_buffer_size(nullptr, pAudioCodecCtx->channels, pAudioCodecCtx->frame_size,
                                                pAudioCodecCtx->sample_fmt, 1) / mSamplePlanes;
        //初始采样缓冲区大小
        mSampleBuffer = new uint8_t*[mSamplePlanes];
        for(int i =0 ; i < mSamplePlanes; i++){
            mSampleBuffer[i] = (uint8_t*)av_malloc((size_t)mSampleSize);
            if(mSampleBuffer[i] == nullptr){
                LOGE("failed to allocate sample buffer");
                return -1;
            }
        }

        //创建音频重采样上下文
        pSampleConvertCtx = swr_alloc_set_opts(pSampleConvertCtx,
                pAudioCodecCtx->channel_layout,
                pAudioCodecCtx->sample_fmt,
                pAudioCodecCtx->sample_rate,
                av_get_default_channel_layout(mChannels),
                mSampleFormat, mSampleRate, 0, nullptr);

        if(!pSampleConvertCtx){
            LOGE("failed to allocate SwrContext");
        } else if(swr_init(pSampleConvertCtx) < 0){
            LOGE("failed to init swr");
        }
    }

    //打印信息
    av_dump_format(pFormatCtx, 0, mDstUrl, 1);

    if(!(pFormatCtx->oformat->flags & AVFMT_NOFILE)){
        if((ret - avio_open(&pFormatCtx->pb, mDstUrl, AVIO_FLAG_WRITE)) < 0){
            LOGE("failed to open output %s", mDstUrl);
            return ret;
        }
    }

    //写入文件头部信息
    ret = avformat_write_header(pFormatCtx, nullptr);
    if(ret < 0){
        LOGE("failed to call avformat_write_header %s", av_err2str(ret));
        return ret;
    }
    return ret;
}

int AVMediaWriter::openEncoder(AVMediaType type) {
    if(type != AVMEDIA_TYPE_VIDEO && type != AVMEDIA_TYPE_AUDIO){
        LOGE("open encoder type failed!");
        return -1;
    }

    int ret;
    AVCodecContext *codecCtx = nullptr;
    AVStream *stream = nullptr;
    AVCodec *encoder = nullptr;
    const char *encodeName = nullptr;

    //根据指定的编码器名称查找编码器
    if(type == AVMEDIA_TYPE_AUDIO){
        encodeName = mAudioEncodeName;
    } else {
        encodeName = mVideoEncodeName;
    }

    if(encodeName != nullptr){
        encoder = avcodec_find_encoder_by_name(encodeName);
    }

    if(encoder == nullptr){
        if(encodeName != nullptr){
            LOGE("failed to find encode by name : %s", encodeName);
        }

        if(type == AVMEDIA_TYPE_VIDEO){
            encoder = avcodec_find_encoder(mVideoCodecId);
        } else {
            encoder = avcodec_find_encoder(mAudioCodecId);
        }
    }

    if(encoder == nullptr){
        LOGE("failed to find encoder");
        return AVERROR_INVALIDDATA;
    }

    //创建编码器上下文
    codecCtx = avcodec_alloc_context3(encoder);
    if(codecCtx == nullptr){
        LOGE("faild to allocted the encodr context");
        return AVERROR(ENOMEM);
    }

    //创建媒体流
    stream = avformat_new_stream(pFormatCtx, encoder);
    if(stream == nullptr){
        LOGE("failed allocted stream");
        return  -1;
    }

    //处理参数
    if (type == AVMEDIA_TYPE_VIDEO){
        codecCtx->width = mWidth;
        codecCtx->height = mHeight;
        codecCtx->pix_fmt = mPixelFormat;
        //got_size 即单位时间内画面的张数，也就是帧率，关键帧最大间隔数
        codecCtx->gop_size = mFrameRate;

        //设置是否时间时间戳作为pts，两种timebase不一样
        if(mUseTimeStamp){
            codecCtx->time_base = (AVRational){1, 1000};
        } else{
            codecCtx->time_base = (AVRational){1, mFrameRate};
        }

        //设置最大比特率
        if(mMaxBitRate > 0){
            codecCtx->rc_max_rate = mMaxBitRate;
            codecCtx->rc_buffer_size = (int)mMaxBitRate;
        }

        auto it = mVideoMetadata.begin();
        for(; it != mVideoMetadata.end(); it++){
            av_dict_set(&stream->metadata, (*it).first.c_str(), (*it).second.c_str(), 0);
        }
    }else{
        codecCtx->sample_rate = mSampleRate;
        codecCtx->channels = mChannels;
        codecCtx->channel_layout = (uint64_t)av_get_default_channel_layout(mChannels);
        codecCtx->sample_fmt = encoder->sample_fmts[0];
        codecCtx->time_base = AVRational{1, codecCtx->sample_rate};
    }

    stream->time_base = codecCtx->time_base;
    //设置编码器全局信息 将一些附加信息SPS/PPS数据放在extradata里面  而不是放在每帧头里
    if(pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER){
        codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    //获取自定义编码参数
    AVDictionary *option = nullptr;
    auto it = mEncodeOptions.begin();
    for(; it != mEncodeOptions.end(); it++){
        av_dict_set(&option, (*it).first.c_str(), (*it).second.c_str(), 0);
    }

    //打开编码器
    ret = avcodec_open2(codecCtx, encoder, &option);
    if(ret < 0){
        LOGE("could not open %s codec: %s", type == AVMEDIA_TYPE_VIDEO ? "video" : "audio", av_err2str(ret));
        av_dict_free(&option);
        return ret;
    }
    av_dict_free(&option);

    //将编码器参数复制到媒体流中
    ret = avcodec_parameters_from_context(stream->codecpar, codecCtx);
    if(ret < 0){
        LOGE("failed to copy encoder paramters to video stream");
        return ret;
    }

    //编订编码器和媒体流
    if(type == AVMEDIA_TYPE_VIDEO){
        pVideoCodecCtx = codecCtx;
        pVideoStream = stream;
    } else{
        pAudioStream = stream;
        pAudioCodecCtx = codecCtx;
    }

    return ret;
}


void AVMediaWriter::reset() {
    mDstUrl = nullptr;
    mWidth = 0;
    mHeight = 0;
    mFrameRate = 0;
    mPixelFormat = AV_PIX_FMT_NONE;
    mVideoCodecId = AV_CODEC_ID_NONE;
    mVideoEncodeName = nullptr;
    mUseTimeStamp = false;
    mHasVideo = false;

    mSampleRate = 0;
    mChannels = 0;
    mSampleFormat = AV_SAMPLE_FMT_NONE;
    mAudioEncodeName = nullptr;
    mAudioCodecId = AV_CODEC_ID_NONE;
    mHasAudio = false;

    pFormatCtx = nullptr;
    pVideoCodecCtx = nullptr;
    pAudioCodecCtx = nullptr;
    pVideoStream = nullptr;
    pAudioStream = nullptr;

    mNbSamples = 0;
    mSampleFrame = nullptr;
    mSampleBuffer = nullptr;
    mSampleSize = 0;
    mSamplePlanes = 0;
    pSampleConvertCtx = nullptr;

    mImageFrame = nullptr;
    mImageBuffer = nullptr;
    mImageCount = 0;
    mStartPts = 0;
    mLastPts = -1;
}

void AVMediaWriter::release() {
    if(mImageFrame != nullptr){
        av_frame_free(&mImageFrame);
        mImageFrame = nullptr;
    }

    if(mImageBuffer != nullptr){
        av_free(mImageBuffer);
        mImageBuffer = nullptr;
    }

    if(mSampleFrame != nullptr){
        av_frame_free(&mSampleFrame);
        mSampleFrame = nullptr;
    }

    if(mSampleBuffer != nullptr){
        for(int i = 0; i < mSamplePlanes; i++){
            av_free(mSampleBuffer[i]);
            mSampleBuffer[i] = nullptr;
        }
        delete[] mSampleBuffer;
        mSampleBuffer = nullptr;
    }

    if(pVideoCodecCtx != nullptr){
        avcodec_free_context(&pVideoCodecCtx);
        pVideoCodecCtx = nullptr;
    }

    if(pAudioCodecCtx != nullptr){
        avcodec_free_context(&pAudioCodecCtx);
        pAudioCodecCtx = nullptr;
    }

    if(pFormatCtx && !(pFormatCtx->oformat->flags & AVFMT_NOFILE)){
        avio_closep(&pFormatCtx->pb);
        avformat_close_input(&pFormatCtx);
        pFormatCtx = nullptr;
    }

    if(pSampleConvertCtx != nullptr){
        swr_free(&pSampleConvertCtx);
        pSampleConvertCtx = nullptr;
    }

    if(pVideoStream != nullptr && pVideoStream->metadata != nullptr){
        av_dict_free(&pVideoStream->metadata);
        pVideoStream->metadata = nullptr;
    }
    pVideoStream = nullptr;
    pAudioStream = nullptr;
}

void AVMediaWriter::setMUseTimeStamp(bool mUseTimeStamp) {
    AVMediaWriter::mUseTimeStamp = mUseTimeStamp;
}

void AVMediaWriter::setMMaxBitRate(int64_t mMaxBitRate) {
    AVMediaWriter::mMaxBitRate = mMaxBitRate;
}

int AVMediaWriter::encodeMediaData(AVMediaData *mediaData) {
    return encodeMediaData(mediaData, nullptr);
}

int AVMediaWriter::encodeMediaData(AVMediaData *mediaData, int *gotFrame) {
    int ret =0, gotFrameLocal;
    if(!gotFrame){
        gotFrame = &gotFrameLocal;
    }
    *gotFrame = 0;

    bool isVideo = (mediaData->type == MediaVideo);
    AVCodecContext *codecCtx = isVideo ? pVideoCodecCtx : pAudioCodecCtx;
    AVStream* stream = isVideo ? pVideoStream : pAudioStream;
    AVFrame* frame = isVideo ? mImageFrame : mSampleFrame;
    uint8_t * data = isVideo ? mediaData->image : mediaData->sample;
    const char *type = isVideo ? "video" : "audio";

    //判断是否支持编码
    if((isVideo && !mHasVideo) || (!isVideo && !mHasAudio)){
        LOGE("no support current type : %s", type);
        return 0;
    }

    if(data != nullptr){
        ret = isVideo ? fillImage(mediaData) : fillSample(mediaData);
        if (ret < 0){
            return -1;
        }
    }

    //初始化数据包
    AVPacket packet;
    packet.data = nullptr;
    packet.size = 0;
    av_init_packet(&packet);
    //编码
    ret = avcodec_send_frame(codecCtx, frame);
    if (ret< 0){
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            return 0;
        }
        LOGE("failed to call avcodec_send_frame: %s", av_err2str(ret));
        return ret;
    }

    while (ret >= 0){
        ret = avcodec_receive_packet(codecCtx, &packet);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            break;
        } else if(ret < 0){
            LOGE("Failed to call avcodec_receive_packet: %s, type: %s", av_err2str(ret), type);
            return ret;
        }

        //计算输出pts
        av_packet_rescale_ts(&packet, codecCtx->time_base, stream->time_base);
        packet.stream_index = stream->index;

        //写入文件
        ret = av_interleaved_write_frame(pFormatCtx, &packet);
        if(ret < 0){
            LOGE("failed to call av_interleaved_write_frame %s type %s", av_err2str(ret), type);
            return ret;
        }
        *gotFrame = 1;
    }

    return 0;
}

int AVMediaWriter::fillImage(AVMediaData *data) {
    int ret = 0;
    ret = av_image_fill_arrays(mImageFrame->data, mImageFrame->linesize, data->image,
            getPixelFormat((PixelFormat)data->pixelFormat), data->width, data->height, 1);
    if(ret < 0){
        LOGE("av_image_fill_arrays error: %s, [%d, %d, %s], [%d, %d], [%d, %d, %s]",
             av_err2str(ret), mImageFrame->width, mImageFrame->height,
             av_get_pix_fmt_name((AVPixelFormat) mImageFrame->format), mWidth, mHeight,
             data->width, data->height, av_get_pix_fmt_name(getPixelFormat((PixelFormat)data->pixelFormat)));
        return -1;
    }

    if(!mUseTimeStamp){
        mImageFrame->pts = mImageCount++;
    } else{
        if(mStartPts == 0){
            mImageFrame->pts = 0;
            mStartPts = data->pts;
        } else{
            mImageFrame->pts = data->pts - mStartPts;
        }

        if(mImageFrame->pts == mLastPts){
            mImageFrame->pts += 10;
        }
        mLastPts = mImageFrame->pts;
    }

    return 0;
}

int AVMediaWriter::fillSample(AVMediaData *data) {
    int ret;
    if(pAudioCodecCtx->channels != mChannels || pAudioCodecCtx->sample_fmt != mSampleFormat ||
            pAudioCodecCtx->sample_rate != mSampleRate){
        ret = swr_convert(pSampleConvertCtx, mSampleBuffer, pAudioCodecCtx->frame_size,
                          (const uint8_t**)&data->sample, pAudioCodecCtx->frame_size);
        if(ret <= 0){
            LOGE("swr_convert error : %s", av_err2str(ret));
            return -1;
        }

        avcodec_fill_audio_frame(mSampleFrame, mChannels, pAudioCodecCtx->sample_fmt,
                                 mSampleBuffer[0], mSampleSize, 0);
        for(int i = 0; i < mSamplePlanes; i++){
            mSampleFrame->data[i] = mSampleBuffer[i];
            mSampleFrame->linesize[i] = mSampleSize;
        }
    } else{
        ret = av_samples_fill_arrays(mSampleFrame->data, mSampleFrame->linesize, data->sample,
                                        pAudioCodecCtx->channels, mSampleFrame->nb_samples,
                                        pAudioCodecCtx->sample_fmt, 1);
    }

    if(ret < 0){
        LOGE("failed to call av_samples_fill_arrays: %s", av_err2str(ret));
        return -1;
    }
    mSampleFrame->pts = mNbSamples;
    mNbSamples += mSampleFrame->nb_samples;

    return 0;
}

int AVMediaWriter::stop() {
    int ret = 0, gotFrame;
    LOGI("flushing video encoder");
    AVMediaData *data = new AVMediaData();
    if(mHasVideo){
        data->type = MediaVideo;
        while (true){
            ret = encodeMediaData(data, &gotFrame);
            if(ret < 0 || !gotFrame){
                break;
            }
        }
    }

    if(mHasAudio){
        data->type = MediaAudio;
        while (true){
            ret = encodeMediaData(data, &gotFrame);
            if(ret < 0 || !gotFrame){
                break;
            }
        }
    }

    delete data;
    av_write_trailer(pFormatCtx);

    return ret;
}
