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
public:


    CTLivePush(const char *liveUrl, CTJNICall *pJniCall);

    ~CTLivePush();

    void initConnect();

    void release();
};


#endif //D97_LIVEPUSH_CTLIVEPUSH_H
