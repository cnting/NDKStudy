//
// Created by cnting on 2023/9/7.
//

#ifndef D3_FFMPEG_CTAUDIO_H
#define D3_FFMPEG_CTAUDIO_H


#include "CTJNICall.h"

extern "C" {
#include "includes/libavformat/avformat.h"
#include "includes/libavcodec/avcodec.h"
#include "includes/libswresample/swresample.h"
};

class CTAudio {
public:
    AVFormatContext *pFormatContext = NULL;
    AVCodecContext *pCodecContext = NULL;
    SwrContext *swrContext = NULL;
    uint8_t *resampleOutBuffer;
    char *url = NULL;
    CTJNICall *pJniCall = NULL;
    int audioStreamIndex = -1;
public:
    CTAudio(int audioStreamIndex, CTJNICall *pJniCall, AVCodecContext *pCodecContext,
            AVFormatContext *pFormatContext,SwrContext *swrContext);

    void play();

    int resampleAudio();

    void initCreateOpenSLES();
};


#endif //D3_FFMPEG_CTAUDIO_H
