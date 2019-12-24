//
// Created by jackzhous on 2019/8/15.
//

#include "AudioDecoder.h"

AudioDecoder::AudioDecoder(AVCodecContext *codecCtx, AVStream *stream, int streamIndex,
                           PlayerStatus *status): MediaDecoder(codecCtx, stream, streamIndex, status) {
    packetAgain = false;
    packet = av_packet_alloc();
    abort = false;
}


AudioDecoder::~AudioDecoder() {
    packetAgain = false;
    if(packet){
        av_packet_free(&packet);
        av_freep(&packet);
        packet = NULL;
    }
}


//采取函数调用方式，不一直循环
int AudioDecoder::getAudioFrame(AVFrame* frame) {
    int ret = 0;
    AVPacket pkt;
    int got_frame=0;

    if(!frame){
        return AVERROR(ENOMEM);
    }

    while (!abort){
        //packet中是否有缓存数据
        if(packetAgain){
            av_packet_move_ref(&pkt, packet);
            packetAgain = false;
        } else{
            ret = queue->pullPacket(&pkt);
            if(ret != PLAYER_OK){
                continue;
            }
        }

        ret = avcodec_send_packet(pCodecCtx, &pkt);
        if(ret < 0){
            if(ret == AVERROR(EAGAIN)){
                av_packet_move_ref(packet, &pkt);
                packetAgain = true;
            } else{
                av_packet_unref(&pkt);
                packetAgain = false;
            }
            continue;
        }

        ret = avcodec_receive_frame(pCodecCtx, frame);
        av_packet_unref(packet);
        av_packet_unref(&pkt);
        if(ret < 0){
            av_frame_unref(frame);
            got_frame = 0;
            continue;
        } else{
            //更新frame的pts，转化为秒
            got_frame = 1;
            AVRational tb = AVRational{1, frame->sample_rate};
            if(frame->pts != AV_NOPTS_VALUE){
                frame->pts = av_rescale_q(frame->pts, av_codec_get_pkt_timebase(pCodecCtx), tb);
            } else if(last_pts != AV_NOPTS_VALUE){
                frame->pts = av_rescale_q(last_pts, last_tb, tb);
            }

            if(frame->pts != AV_NOPTS_VALUE){
                last_pts = frame->pts + frame->nb_samples;
                last_tb = tb;
            }
            break;
        }
    }

    if(ret < 0){
        return -1;
    }
    return got_frame;


}