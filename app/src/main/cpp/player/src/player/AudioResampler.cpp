//
// Created by jackzhous on 2019-09-19.
//

#include <device/AudioDevice.h>
#include "AudioResampler.h"

AudioResampler::AudioResampler(PlayerStatus *status, AudioDecoder *audioDecoder, MediaSync *sync) {
    this->playerStatus = status;
    this->decoder = audioDecoder;
    this->sync = sync;
    audioState = (AudioState *)av_mallocz(sizeof(AudioState));
    memset(audioState, 0, sizeof(AudioState));
    frame = av_frame_alloc();
}


AudioResampler::~AudioResampler() {
    playerStatus = NULL;
    decoder = NULL;
    sync = NULL;
    if(audioState){
        swr_free(&audioState->swrCtx);
        av_freep(&audioState->resampleBuffer);
        memset(audioState, 0 , sizeof(AudioState));
        av_freep(audioState);
        audioState = NULL;
    }

    if(frame){
        av_frame_unref(frame);
        av_frame_free(&frame);
        frame = NULL;
    }
}


int AudioResampler::setResampleParams(AudioDeviceSpec *spec, int64_t wanted_channel_layout) {
    audioState->audioParamsSrc = audioState->audioParamsTarget;
    audioState->audio_hw_buf_size = spec->size;
    audioState->buffersize = 0;
    audioState->bufferIndex = 0;
    audioState->audio_diff_avg_coef = (exp(log(0.01)) / AUDIO_DIFF_AVG_NB);
    audioState->audio_diff_avg_count = 0;
    audioState->audio_diff_threshold = (double)(audioState->audio_hw_buf_size) / audioState->audioParamsTarget.bytes_per_sec;

    audioState->audioParamsTarget.fmt = AV_SAMPLE_FMT_S16;
    audioState->audioParamsTarget.freq = spec->freq;
    audioState->audioParamsTarget.channel_layout = wanted_channel_layout;
    audioState->audioParamsTarget.channels = spec->channels;
    audioState->audioParamsTarget.frame_size = av_samples_get_buffer_size(NULL, audioState->audioParamsTarget.channels, 1, audioState->audioParamsTarget.fmt, 1);
    audioState->audioParamsTarget.bytes_per_sec = av_samples_get_buffer_size(NULL, audioState->audioParamsTarget.channels,
                                                                             audioState->audioParamsTarget.freq,
                                                                             audioState->audioParamsTarget.fmt, 1);

    if(audioState->audioParamsTarget.bytes_per_sec <= 0 || audioState->audioParamsTarget.frame_size <= 0){
        av_log(NULL, AV_LOG_ERROR, "av_sample_get_buffer_size failed\n");
        return -1;
    }

    return 0;
}


void AudioResampler::pcmQueueCallback(uint8_t *stream, int len) {
    int bufferSize, length;

    //没有音频解码器时，直接返回
    if(!decoder){
        memset(stream, 0, len);
        return;
    }
    //bufferIndex数据开始的位置（会变）  bufferSize结束的位置（不变）  outBuffer读取的位置（会变）
    audioState->audio_callback_time = av_gettime_relative();
    while (len > 0){
        if(audioState->bufferIndex >= audioState->buffersize){
            bufferSize = audioFrameResample();
            if(bufferSize < 0){
                audioState->outputBuffer = NULL;
                audioState->buffersize = (unsigned int)(AUDIO_MIN_BUFFER_SIZE / audioState->audioParamsTarget.frame_size * audioState->audioParamsTarget.frame_size);
            } else{
                audioState->buffersize = bufferSize;
            }
            audioState->bufferIndex = 0;
        }

        length = audioState->buffersize - audioState->bufferIndex;
        if(length > len){
            length = len;
        }
        //赋值经过转码输出的PCM数据到缓冲区
        if(audioState->outputBuffer != NULL && !playerStatus->mute){
            memcpy(stream, audioState->outputBuffer + audioState->bufferIndex, length);
        } else{
            memset(stream, 0, length);
        }
        len -= length;
        stream += length;
        audioState->bufferIndex += length;
    }

    //还没有写完进去的数据长度
    audioState->writeBufferSize = audioState->buffersize - audioState->bufferIndex;

    if(!isnan(audioState->audioClock) && sync){
        //乘2是因为有两个通道 audioClock在audioFrameResample更新了播放解码后数据的时间，这里要减去这些数据，还没有播放
        sync->updateAudioColock(audioState->audioClock - (double)(2 * audioState->audio_hw_buf_size
        + audioState->writeBufferSize) / audioState->audioParamsTarget.bytes_per_sec,
        audioState->audio_callback_time / 1000000.0);
    }
}


int AudioResampler::audioFrameResample() {
    int data_size, resampled_data_size = 0;
    int64_t dec_channel_layout;
    int wanted_nb_samples;
    int translate_time = 1;
    int ret = -1;

    //处于暂停状态
    if(!decoder || playerStatus->abortRequest || playerStatus->pauseRequest){
        return -1;
    }

    for(;;){
        ret = decoder->getAudioFrame(frame);
        if((ret < 0)){
            return -1;
        }

        if(ret == 0){
            continue;
        }

        data_size = av_samples_get_buffer_size(NULL, av_frame_get_channels(frame),
                                                frame->nb_samples, (AVSampleFormat)frame->format, 1);

        dec_channel_layout = (frame->channel_layout && av_frame_get_channels(frame) == av_get_channel_layout_nb_channels(frame->channel_layout))
                             ? frame->channel_layout : av_get_default_channel_layout(av_frame_get_channels(frame));

        wanted_nb_samples = audioSynchronize(frame->nb_samples);

        //解码后音频帧格式和音频设备播放的不同 进行转换
        if(frame->format != audioState->audioParamsSrc.fmt || dec_channel_layout != audioState->audioParamsSrc.channel_layout
                        || frame->sample_rate != audioState->audioParamsSrc.freq
                        || (wanted_nb_samples != frame->nb_samples && !audioState->swrCtx)){
            swr_free(&audioState->swrCtx);
            //根据转换后和前的格式参数生成上下文结构
            audioState->swrCtx = swr_alloc_set_opts(NULL, audioState->audioParamsTarget.channel_layout,
                                                    audioState->audioParamsTarget.fmt, audioState->audioParamsTarget.freq,
                                                    dec_channel_layout, (AVSampleFormat)frame->format,
                                                    frame->sample_rate, 0 ,NULL);
            //swr生成失败
            if(!audioState->swrCtx || swr_init(audioState->swrCtx) < 0){
                av_log(NULL, AV_LOG_ERROR, "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                       frame->sample_rate,
                       av_get_sample_fmt_name((AVSampleFormat)frame->format),
                       av_frame_get_channels(frame),
                       audioState->audioParamsTarget.freq,
                       av_get_sample_fmt_name(audioState->audioParamsTarget.fmt),
                       audioState->audioParamsTarget.channels);
                swr_free(&audioState->swrCtx);
                return -1;
            }

            audioState->audioParamsSrc.channel_layout = dec_channel_layout;
            audioState->audioParamsSrc.channels = av_frame_get_channels(frame);
            audioState->audioParamsSrc.freq = frame->sample_rate;
            audioState->audioParamsSrc.fmt = (AVSampleFormat)frame->format;
        }

        //音频重采样
        if(audioState->swrCtx){
            const uint8_t **in = (const uint8_t **)frame->extended_data;
            uint8_t **out = &audioState->resampleBuffer;
//            int out_count = wanted_nb_samples * audioState->audioParamsTarget.freq / frame->sample_rate + 256;
            int out_count = av_rescale_rnd(swr_get_delay(audioState->swrCtx, frame->sample_rate) + frame->nb_samples, audioState->audioParamsTarget.freq, frame->sample_rate, AV_ROUND_UP);
            int out_size = av_samples_get_buffer_size(NULL, audioState->audioParamsTarget.channels, out_count, audioState->audioParamsTarget.fmt, 0);
            int len2;
            if(out_size < 0){
                av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
                return -1;
            }

            if(wanted_nb_samples != frame->nb_samples){
                if(swr_set_compensation(audioState->swrCtx, (wanted_nb_samples - frame->nb_samples) * audioState->audioParamsTarget.freq / frame->sample_rate,
                                wanted_nb_samples* audioState->audioParamsTarget.freq / frame->sample_rate) < 0){
                    av_log(NULL, AV_LOG_ERROR, "swr_set_compensation() failed\n");
                    return -1;
                }
            }

            av_fast_malloc(&audioState->resampleBuffer, &audioState->resampleSize, out_size);
            if(!audioState->resampleBuffer){
                return AVERROR(ENOMEM);
            }
            len2 = swr_convert(audioState->swrCtx, out, out_count, in, frame->nb_samples);
            if(len2 < 0){
                av_log(NULL, AV_LOG_ERROR, "swr_convert() failed\n");
                return -1;
            }

            if(len2 == out_count){
                av_log(NULL, AV_LOG_WARNING, "audio buffer is probably too small\n");
                if (swr_init(audioState->swrCtx) < 0) {
                    swr_free(&audioState->swrCtx);
                }
            }
            audioState->outputBuffer = audioState->resampleBuffer;
            resampled_data_size = len2 * audioState->audioParamsTarget.channels * av_get_bytes_per_sample(audioState->audioParamsTarget.fmt);

//            audioState->outputBuffer = frame->data[0];
//            resampled_data_size = data_size;
        }
        break;
    }

    //利用pts更新时钟
    if(frame->pts != AV_NOPTS_VALUE){
        //解码时间戳加这么 nb_samples个采样点的时间
        audioState->audioClock = frame->pts * av_q2d((AVRational){1, frame->sample_rate}) +
                (double)frame->nb_samples / frame->sample_rate;
    } else{
        audioState->audioClock = NAN;
    }

    av_frame_unref(frame);
    return resampled_data_size;
}


int AudioResampler::audioSynchronize(int nbSamples) {
    int wanted_nb_samples = nbSamples;

    //如果始终不同步到音频流，则需要进行对音频同步
    if(playerStatus->syncType != AV_SYNC_AUDIO){
        double diff, avg_diff;
        int min_nb_samples, max_nb_sample;
        diff = sync ? sync->getAudioDffClock() : 0;
        if(!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD){
            audioState->audio_diff_cum = diff + audioState->audio_diff_avg_coef * audioState->audio_diff_cum;
            if(audioState->audio_diff_avg_count < AUDIO_DIFF_AVG_NB){
                audioState->audio_diff_avg_count++;
            } else{
                avg_diff = audioState->audio_diff_cum * (1.0 - audioState->audio_diff_avg_coef);

                if(fabs(avg_diff) >= audioState->audio_diff_threshold){
                    wanted_nb_samples = nbSamples + (int)(diff * audioState->audioParamsSrc.freq);
                    min_nb_samples = ((nbSamples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    max_nb_sample = ((nbSamples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    wanted_nb_samples = av_clip(wanted_nb_samples, min_nb_samples, max_nb_sample);
                }
            }
        } else{
            audioState->audio_diff_avg_count = 0;
            audioState->audio_diff_cum = 0;
        }
    }

    return wanted_nb_samples;
}