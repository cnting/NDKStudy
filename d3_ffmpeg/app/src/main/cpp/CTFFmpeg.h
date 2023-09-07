//
// Created by cnting on 2023/9/7.
//
#ifndef D3_FFMPEG_CTFFMPEG_H
#define D3_FFMPEG_CTFFMPEG_H

extern "C" {
#include "includes/libavformat/avformat.h"
#include "includes/libavcodec/avcodec.h"
#include "includes/libswresample/swresample.h"
}

#include "CTJNICall.h"

class CTFFmpeg {
public:
    AVFormatContext *pFormatContext = NULL;
    AVCodecContext *pCodecContext = NULL;
    SwrContext *swrContext = NULL;
    uint8_t *resampleOutBuffer;
    char *url = NULL;
    CTJNICall *pJniCall = NULL;
public:
    CTFFmpeg(CTJNICall *pJniCall, const char *url);

    ~CTFFmpeg();

    void play();

    void prepare();

    void prepare(ThreadMode threadMode);

    void preparedAsync();

    void callPlayJniError(ThreadMode threadMode, int code, char *msg);

private:
    void release();
};


#endif //D3_FFMPEG_CTFFMPEG_H
