//
// Created by jackzhous on 2019/8/13.
//

#include <device/AudioDevice.h>
#include "Player.h"


Player::Player() {
    av_register_all();
    avformat_network_init();
    playerStatus = new PlayerStatus();
    sync = NULL;
    aDecoder = NULL;
    vDecoder = NULL;
    pFormatCtx = NULL;
    audioResampler = NULL;
    aDevice = new AudioDevice();
    eof = 0;
    mExit = true;
    readPacketThread = NULL;
}


Player::~Player() {
    delete playerStatus;
    delete aDevice;
    aDevice = NULL;
    LOGI("player is destroyed");
    reset();
    avformat_network_deinit();
}

void Player::init() {
    sync = new MediaSync(playerStatus);
}

void Player::setVideoDevice(VideoDevice *device) {
    this->vDevice = device;
}

void Player::reset() {
    stop();
    if(aDecoder != NULL){
        aDecoder->stop();
        delete aDecoder;
        aDecoder = NULL;
    }

    if(vDecoder != NULL){
        vDecoder->stop();
        delete vDecoder;
        vDecoder = NULL;
    }

    if(sync != NULL){
        sync->reset();
        delete sync;
        sync = NULL;
    }
    if(pFormatCtx != NULL){
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = NULL;
    }
    if(playerStatus != NULL){
        playerStatus->reset();
    }

    if(audioResampler != NULL){
        delete audioResampler;
        audioResampler = NULL;
    }

    eof = 0;
    lastPlayerStatus = 0;
    attchmentRequest = false;
}

MessageQueue* Player::getMessageQueue() {
    return playerStatus->queue;
}

void Player::setVideoUrl(char* url) {
    playerStatus->setVideoUrl(url);
    playerStatus->queue->addMessage(SET_VIDEO_DATA, "set video url");
}

void Player::setVideoPath(char *path) {
    playerStatus->setVideoPath(path);
    playerStatus->queue->addMessage(SET_VIDEO_DATA, "set video path");
}


static int ffmpeg_open_input_callback(void *pointer) {
    PlayerStatus* status = (PlayerStatus*)pointer;
    if(status->abortRequest){
        return STOP_WAIT;
    }

    return KEEP_WAIT;
}

int Player::getWidth() {
    return vDecoder->getCodecCtx()->width;
}

void Player::setLoop(int loop) {
    playerStatus->loop = loop;
}

int Player::getHeight() {
    return vDecoder->getCodecCtx()->height;
}

int Player::getRorate() {
    return vDecoder->getRorate();
}

void Player::prepare() {
    AutoMutex lock(mMutex);
    playerStatus->abortRequest = 0;
    if(!readPacketThread){
        readPacketThread = new Thread(this);
        readPacketThread->start();

        LOGI("read Packet Thread start");
    }
}


void Player::startPlay() {
    AutoMutex lock(mMutex);
    playerStatus->abortRequest = 0;
    playerStatus->pauseRequest = 0;
    mExit = false;
    mCond.signal();
}

void Player::pause() {
    AutoMutex lock(mMutex);
    playerStatus->pauseRequest = 1;
    mCond.signal();
}

void Player::onResume() {
    AutoMutex lock(mMutex);
    playerStatus->pauseRequest = 0;
    mCond.signal();
}

void Player::stop() {
    mMutex.lock();
    playerStatus->abortRequest = 1;
    mCond.signal();

    while (!mExit){
        mCond.wait(mMutex);
    }
    mMutex.unlock();
    if(readPacketThread){
        readPacketThread->join();
        delete readPacketThread;
        readPacketThread = NULL;
    }
    LOGI("读取ffmpeg线程结束");
}

/**
 * 编码钱初始化ffmpeg
 */
int Player::prepareFFmpeg() {
    int ret = 0;
    pFormatCtx = avformat_alloc_context();
    //打开输入回调
    pFormatCtx->interrupt_callback.callback = ffmpeg_open_input_callback;
    pFormatCtx->interrupt_callback.opaque = playerStatus;

    char* videoUrl = playerStatus->url == NULL ? playerStatus->vedioPath : playerStatus->url;

    if((ret = avformat_open_input(&pFormatCtx, videoUrl, NULL, NULL)) < 0){
        printError(videoUrl, ret);
        return ret;
    }

    if((ret = avformat_find_stream_info(pFormatCtx, NULL)) < 0){
        printError(videoUrl, ret);
        return ret;
    }

    int audoIndex = -1;
    int videoIndex = -1;
    for(int i = 0; i < pFormatCtx->nb_streams; i++){
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            videoIndex = i;
        } else if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audoIndex = i;
        }
    }

    playerStatus->queue->addMessage(FIND_AV_STREAM, "find av stream from media");
    videoIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, videoIndex, -1, NULL, 0);
    audoIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, audoIndex, -1, NULL, 0);

    if(audoIndex == -1 && videoIndex == -1){
        LOGE("%s founded no streams", videoUrl);
        return ret;
    }

    if(prepareDecoder(audoIndex) > 0 && prepareDecoder(videoIndex) > 0){
        playerStatus->queue->addMessage(PREPARE_DECODER, "prepare decoder success");
    } else{
        return PLAYER_FAILED;
    }
    if(pFormatCtx->duration != AV_NOPTS_VALUE && pFormatCtx->duration > 0){
        //mDuration只是用于外部显示作用以及拉动进度条，当设置视频位置时需要将时间转换回来
        playerStatus->mDuration = av_rescale(pFormatCtx->duration, 1000, AV_TIME_BASE);
    }

    playerStatus->isLiveStream = isLiveStream(pFormatCtx);
    sync->setMaxFrameDuration(pFormatCtx->iformat->flags & AVFMT_TS_DISCONT ? 10.0 : 3600.0);
    return PLAYER_OK;
}

int Player::prepareDecoder(int streamIndex) {
    AVCodecContext* codecContext;
    int ret;
    if(streamIndex < 0 || streamIndex >= pFormatCtx->nb_streams){
        return -1;
    }

    codecContext = avcodec_alloc_context3(NULL);
    if(!codecContext){
        LOGI("create codec ctx failed");
        return AVERROR(ENOENT);

    }
    if((ret = avcodec_parameters_to_context(codecContext, pFormatCtx->streams[streamIndex]->codecpar)) < 0){
        return ret;
    }

    // 设置时钟基准
    av_codec_set_pkt_timebase(codecContext, pFormatCtx->streams[streamIndex]->time_base);

    const char* codecName = NULL;
    switch (codecContext->codec_type){
        case AVMEDIA_TYPE_VIDEO:
            codecName = playerStatus->forcedVideoCodecName;
//            codecName = "h264_mediacodec";
            break;


        case AVMEDIA_TYPE_AUDIO:
            codecName = playerStatus->forcedAduioCodecName;
            break;

        default:
            break;
    }
    AVCodec* codec = NULL;
    if(codecName){
        codec = avcodec_find_decoder_by_name(codecName);
    }

    if(!codec){
        LOGE("find codec: %s by name is failed!", codecName);
        codec = avcodec_find_decoder(codecContext->codec_id);
    }

    if(!codec){
        LOGE("find codec: %d by id is failed!", codecContext->codec_id);
        return -1;
    }
    codecContext->codec_id = codec->id;
    LOGI("codec name %s", codec->name);
// 设置一些播放参数
    int stream_lowres = playerStatus->lowres;
    if (stream_lowres > av_codec_get_max_lowres(codec)) {
        av_log(codecContext, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
               av_codec_get_max_lowres(codec));
        stream_lowres = av_codec_get_max_lowres(codec);
    }
    av_codec_set_lowres(codecContext, stream_lowres);
#if FF_API_EMU_EDGE
    if (stream_lowres) {
        codecContext->flags |= CODEC_FLAG_EMU_EDGE;
    }
#endif
    if (playerStatus->fast) {
        codecContext->flags2 |= AV_CODEC_FLAG2_FAST;
    }
#if FF_API_EMU_EDGE
    if (codec->capabilities & AV_CODEC_CAP_DR1) {
        codecContext->flags |= CODEC_FLAG_EMU_EDGE;
    }
#endif
    AVDictionary *opts = NULL;
//    opts = filterCodecOptions(playerStatus->codec_opts, avctx->codec_id, pFormatCtx, pFormatCtx->streams[streamIndex], codec);
    if (!av_dict_get(opts, "threads", NULL, 0)) {
        av_dict_set(&opts, "threads", "auto", 0);
    }

    if (stream_lowres) {
        av_dict_set_int(&opts, "lowres", stream_lowres, 0);
    }

    if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO || codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
        av_dict_set(&opts, "refcounted_frames", "1", 0);
    }
    ret = avcodec_open2(codecContext, codec, &opts);
    if(ret < 0){
        printError("open codec failed", ret);
        return ret;
    }

    switch (codecContext->codec_type){
        case AVMEDIA_TYPE_VIDEO:
            vDecoder = new VideoDecoder(codecContext, pFormatCtx->streams[streamIndex], streamIndex, playerStatus, pFormatCtx);
            break;


        case AVMEDIA_TYPE_AUDIO:
            aDecoder = new AudioDecoder(codecContext, pFormatCtx->streams[streamIndex], streamIndex, playerStatus);
            break;

        default:
            return -1;
    }
    return PLAYER_OK;
}



void Player::run() {
    //1. 初始化ffmpeg库
    init();
    int ret = prepareFFmpeg();
    if(ret <= PLAYER_FAILED){
        playerStatus->queue->addMessage(FFMPEG_INIT_FAILED, "ffmpeg init failed");
        return;
    }
    playerStatus->queue->addMessage(PREPARE_PLAYER, "player is prepared!");
    /**
     * 这里需要做的工作主要有
     * 2. 打开音视频设备
     * 3. 准备音视频解码线程
     * 4. 音视频同步启动
     */
    //开启音视频解码线程
    if(vDecoder != NULL && aDecoder != NULL){
        vDecoder->start();
        aDecoder->start();
        playerStatus->queue->addMessage(START_VIDEO_DECODER, "start video decoder thread");
        playerStatus->queue->addMessage(START_AUDIO_DECODER, "start audio decoder thread");
    }

    //打开音频设备
    if(aDecoder){
        AVCodecContext* ctx = aDecoder->getCodecCtx();
        ret = openAudioDevice(ctx->channel_layout, ctx->channels, ctx->sample_rate);
        if(ret < 0){
            LOGE("open audio Device failed");
            if(playerStatus->syncType == AV_SYNC_AUDIO){
                if(vDecoder){
                    playerStatus->syncType = AV_SYNC_VIDEO;
                } else{
                    playerStatus->syncType = AV_SYNC_EXTERNAL;
                }
            }
        } else{
            aDevice->start();
        }
    }

    if(vDecoder){
        if(playerStatus->syncType == AV_SYNC_AUDIO){
            vDecoder->setMasterClock(sync->getAudioClock());
        } else if(playerStatus->syncType == AVMEDIA_TYPE_VIDEO){
            vDecoder->setMasterClock(sync->getVideoClock());
        } else{
            vDecoder->setMasterClock(sync->getExtClock());
        }

    }

    if(sync){
        sync->setVideoDevice(vDevice);
        sync->start(aDecoder, vDecoder);
    }
    //卡主  等到java重新设置surface大小
    while (mExit){
        av_usleep(10 * 1000);
    }

    LOGI("prepare ffmpeg!");
    AVPacket packet;
    //循环读取packet
    for (;;) {
        if(playerStatus->abortRequest){
            playerStatus->queue->addMessage(EXIT_PLAYER, "exit player");
            LOGI("exit player");
            break;
        }

        //暂停 - 继续
        if(playerStatus->pauseRequest != lastPlayerStatus){
            if(playerStatus->pauseRequest){
                av_read_pause(pFormatCtx);
            } else{
                av_read_play(pFormatCtx);
            }
            lastPlayerStatus = playerStatus->pauseRequest;
        }

        /**
         * 视频定位
         */
        if(playerStatus->seekRequest){
            int64_t min = playerStatus->seekPos - 2;
            int64_t max = playerStatus->seekPos + 2;
            ret = avformat_seek_file(pFormatCtx, -1, min, playerStatus->seekPos, max, playerStatus->seekFlag);
            if(ret < 0){
                LOGE("seek file failed");
            } else{
                //时钟同步，解码器刷新
                vDecoder->flush();
                aDecoder->flush();
                sync->updateExternalClock(playerStatus->seekPos / AV_TIME_BASE);
            }
            playerStatus->seekRequest = 0;
            attchmentRequest = true;
            playerStatus->queue->addMessage(SEEK_END, "seek file finish");
            mCond.signal();
        }

        if(attchmentRequest){
            //有封面包数据
            if(vDecoder && vDecoder->getStream()->disposition & AV_DISPOSITION_ATTACHED_PIC){
                AVPacket pkt;
                ret = av_copy_packet(&pkt, &vDecoder->getStream()->attached_pic);
                if(ret < 0){
                    break;
                }
                vDecoder->pushAVPacket(&pkt);
            }
            attchmentRequest = 0;
        }

        if(vDecoder->getMemorySize() + aDecoder->getMemorySize() > MAX_QUEUE_SIZE ||
           (vDecoder->hasEnoughPackets() && aDecoder->hasEnoughPackets())){
            continue;
        }
        //不是读出包来就丢去解码，需要考虑到一些问题
        /**
         * 1. Decoder里面的packet存的数量足够多了不要继续添加
         * 2. 如果有拖拽的视频进度的情况，拖拽前的包无须送入解码器解码
         * 3.
         */
        ret = av_read_frame(pFormatCtx, &packet);
        if(ret < 0){
            //读取视频完成，但是这里不一定播完
            if(ret == AVERROR_EOF || avio_feof(pFormatCtx->pb) && !eof){
                eof = 1;
            }

            if(pFormatCtx->pb->error){
                printError("read packet", pFormatCtx->pb->error);
                LOGE("read packet error!");
                break;
            }

            //查看数据包是否读完
            if(aDecoder->getPacketQueue()->getPacketLen() == 0 && vDecoder->getPacketQueue()->getPacketLen() == 0){
                if(playerStatus->loop){
                    seekVideo(playerStatus->start_time == AV_NOPTS_VALUE ? 0 : playerStatus->start_time);
                } else{
                    break;
                }
            }
            // 读取失败时，睡眠10毫秒继续
            av_usleep(10 * 1000);
            continue;
        } else{
            eof = 0;
        }
        int pktIndex = packet.stream_index;
        if(pktIndex == vDecoder->getStreamIndex()){
            vDecoder->getPacketQueue()->pushPacket(&packet);
        } else if(pktIndex == aDecoder->getStreamIndex()){
            aDecoder->getPacketQueue()->pushPacket(&packet);

        } else{
            av_packet_unref(&packet);
        }
    }

    if(aDecoder != NULL){
        aDecoder->stop();
    }

    if(vDecoder != NULL){
        vDecoder->stop();
    }

    if(aDevice != NULL){
        aDevice->stop();
    }

    if(sync != NULL){
        sync->stop();
    }
    mExit = true;
    mCond.signal();
    playerStatus->queue->addMessage(STOP_PLAER, "player stoped");
}

/**
 * times是绝对时间
 */
void Player::seekVideo(float times) {
    //不可跳转
    if(times == AV_NOPTS_VALUE){
        return;
    }
    mMutex.lock();
    while (playerStatus->seekRequest){
        mCond.wait(mMutex);
    }
    mMutex.unlock();
    int64_t start_time = pFormatCtx->start_time < 0 ? 0 : pFormatCtx->start_time;
    int64_t pos = av_rescale(times, AV_TIME_BASE, 1000);
    playerStatus->seekPos = start_time + pos;
    playerStatus->seekRequest = 1;
    playerStatus->seekFlag &= ~AVSEEK_FLAG_BYTE;
    mCond.signal();
}


void audioPCMQueueCallback(void *opaque, uint8_t *stream, int len) {
    Player *mediaPlayer = (Player *) opaque;
    mediaPlayer->pcmQueueCallback(stream, len);
}


void Player::pcmQueueCallback(uint8_t *stream, int len) {
    if(!audioResampler){
        memset(stream, 0, len);
        return;
    }
    audioResampler->pcmQueueCallback(stream, len);
//    if(playerStatus->queue && playerStatus->syncType != AV_SYNC_VIDEO){
//        playerStatus->queue->addMessage(MSG_CURRENT_POSITON, "audio callback is prepared!");
//    }
}

int Player::openAudioDevice(int64_t wanted_channel_layout, int wanted_nb_channels,
                            int wanted_sample_rate) {
    AudioDeviceSpec wanted_spec, spec;
    const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    const int next_sample_rates[] = {44100, 48000};
    int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;       //数组长度-1
    //通道数不一致
    if(wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout) || !wanted_channel_layout){
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
        wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }
    wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
    wanted_spec.channels = wanted_nb_channels;
    wanted_spec.freq = wanted_sample_rate;
    if(wanted_spec.freq <= 0 || wanted_spec.channels <= 0){
        LOGE("audio spec freq %d, channel %d", wanted_spec.freq, wanted_spec.channels);
        return -1;
    }
    //选择合适的频率
    while(next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq){
        next_sample_rate_idx--;
    }

    wanted_spec.format = AV_SAMPLE_FMT_S16;
    //这不计算挺复杂的  但是没用  不知道为什么要写在这儿
    wanted_spec.samples = FFMAX(AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wanted_spec.freq / AUDIO_MAX_CALLBACKS_PER_SEC));
    wanted_spec.callback = audioPCMQueueCallback;
    wanted_spec.userData = this;

    //打开音频设备
    while(aDevice->open(&wanted_spec, &spec) < 0){
        av_log(NULL, AV_LOG_WARNING, "Failed to open audio device: (%d channels, %d Hz)!\n",
               wanted_spec.channels, wanted_spec.freq);
        wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
        if (!wanted_spec.channels) {
            wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
            wanted_spec.channels = wanted_nb_channels;
            if (!wanted_spec.freq) {
                av_log(NULL, AV_LOG_ERROR, "No more combinations to try, audio open failed\n");
                return -1;
            }
        }
        wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
    }

    if(spec.format != AV_SAMPLE_FMT_S16){
        av_log(NULL, AV_LOG_ERROR, "audio format %d is not supported!\n", spec.format);
        return -1;
    }

    if(spec.channels != wanted_spec.channels){
        wanted_channel_layout = av_get_default_channel_layout(spec.channels);
        if (!wanted_channel_layout) {
            av_log(NULL, AV_LOG_ERROR, "channel count %d is not supported!\n", spec.channels);
            return -1;
        }
    }

    //初始化音频重采样器
    if(!audioResampler){
        audioResampler = new AudioResampler(playerStatus, aDecoder, sync);
    }
    audioResampler->setResampleParams(&spec, wanted_channel_layout);
    return spec.size;
}