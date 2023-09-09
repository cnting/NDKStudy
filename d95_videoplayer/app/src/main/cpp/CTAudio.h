//
// Created by cnting on 2023/9/7.
//

#ifndef D3_FFMPEG_CTAUDIO_H
#define D3_FFMPEG_CTAUDIO_H


#include "CTJNICall.h"
#include "CTPacketQueue.h"
#include "CTPlayerStatus.h"
#include "CTMedia.h"

extern "C" {
#include "includes/libavformat/avformat.h"
#include "includes/libavcodec/avcodec.h"
#include "includes/libswresample/swresample.h"
};

class CTAudio : public CTMedia {
public:
    SwrContext *swrContext = NULL;
    uint8_t *resampleOutBuffer;
public:
    CTAudio(int audioStreamIndex, CTJNICall *pJniCall,
            CTPlayerStatus *pPlayStatus);

    ~CTAudio();

    void play();

    int resampleAudio();

    void initCreateOpenSLES();

    void privateAnalysisStream(ThreadMode threadMode, AVFormatContext* pFormatContext);

    void release();
};


#endif //D3_FFMPEG_CTAUDIO_H
