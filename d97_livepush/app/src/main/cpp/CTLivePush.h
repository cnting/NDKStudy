//
// Created by cnting on 2023/9/11.
//

#ifndef D97_LIVEPUSH_CTLIVEPUSH_H
#define D97_LIVEPUSH_CTLIVEPUSH_H


#include "CTJNICall.h"
#include "CTPacketQueue.h"

class CTLivePush {
public:
    CTJNICall *pJniCall = NULL;
    char *liveUrl = NULL;
    CTPacketQueue *pPacketQueue = NULL;
    RTMP *rtmp = NULL;
    bool isPushing = true;
    uint32_t startTime;
    pthread_t initConnectTid;
public:


    CTLivePush(const char *liveUrl, CTJNICall *pJniCall);

    ~CTLivePush();

    void initConnect();

    void release();

    void pushSpsPps(jbyte *spsData, jint spsLength, jbyte *ppsData, jint ppsLength);


    void pushVideo(jbyte *videoData, jint dataLen, jboolean keyFrame);


    void pushAudio(jbyte *audioData, jint dataLen);

    void stop();
};


#endif //D97_LIVEPUSH_CTLIVEPUSH_H
