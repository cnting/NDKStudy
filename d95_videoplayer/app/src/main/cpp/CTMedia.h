//
// Created by cnting on 2023/9/9.
//

#ifndef D95_VIDEOPLAYER_CTMEDIA_H
#define D95_VIDEOPLAYER_CTMEDIA_H

#include "CTJNICall.h"
#include "CTPacketQueue.h"
#include "CTPlayerStatus.h"

extern "C" {
#include "includes/libavcodec/avcodec.h"
};

class CTMedia {
public:
    int streamIndex = -1;
    AVCodecContext *pCodecContext = NULL;
    CTJNICall *pJniCall = NULL;
    CTPacketQueue *pPacketQueue = NULL;
    CTPlayerStatus *pPlayerStatus = NULL;
    double currentTime = 0;
    double lastUpdateTime = 0;
    //时间基
    AVRational timeBase;
    int64_t duration = 0;
public:
    CTMedia(int streamIndex, CTJNICall *pJniCall, CTPlayerStatus *pPlayerStatus);

    ~CTMedia();

public:
    virtual void play() = 0;

    void analysisStream(ThreadMode threadMode, AVFormatContext* pFormatContext);

    virtual void privateAnalysisStream(ThreadMode threadMode, AVFormatContext* pFormatContext) = 0;

    virtual void release() = 0;

    void callPlayJniError(ThreadMode threadMode, int code, char *msg);

private:
    void publicAnalysisStream(ThreadMode threadMode, AVFormatContext* pFormatContext);

};


#endif //D95_VIDEOPLAYER_CTMEDIA_H
