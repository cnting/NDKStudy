//
// Created by cnting on 2023/9/7.
//

#ifndef D3_FFMPEG_CTAUDIO_H
#define D3_FFMPEG_CTAUDIO_H


#include "CTJNICall.h"
#include "CTPacketQueue.h"
#include "CTPlayerStatus.h"

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
    CTPacketQueue *pPacketQueue = NULL;
    CTPlayerStatus *pPlayerStatus = NULL;
    pthread_t readPacketThreadT;
public:
    CTAudio(int audioStreamIndex, CTJNICall *pJniCall, AVFormatContext *pFormatContext);

    ~CTAudio();

    void play();

    void stop();

    int resampleAudio();

    void initCreateOpenSLES();

    void analysisStream(ThreadMode threadMode);

    void callPlayJniError(ThreadMode threadMode, int code, char *msg);

    void release();
};


#endif //D3_FFMPEG_CTAUDIO_H
