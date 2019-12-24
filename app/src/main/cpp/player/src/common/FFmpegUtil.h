//
// Created by jackzhous on 2019/8/14.
//

#ifndef MYPLAYER_FFMPEGUTIL_H
#define MYPLAYER_FFMPEGUTIL_H


#include <AndroidLog.h>
#include <string.h>

extern "C"{

#include <libavutil/dict.h>
#include <libavutil/eval.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
};


void printError(const char* name, int error);

bool isLiveStream(AVFormatContext *s);
int checkStreamSpecifier(AVFormatContext *s, AVStream *st, const char *spec);

AVDictionary *filterCodecOptions(AVDictionary *opts, enum AVCodecID codec_id,
                                 AVFormatContext *s, AVStream *st, AVCodec *codec);

#endif //MYPLAYER_FFMPEGUTIL_H
