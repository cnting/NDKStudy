//
// Created by cnting on 2023/9/11.
//

#include <cstring>
#include <cstdlib>
#include "CTLivePush.h"
#include "constants.h"

CTLivePush::CTLivePush(const char *liveUrl, CTJNICall *pJniCall) {
    this->pJniCall = pJniCall;
    //复制一份，避免外面销毁
    this->liveUrl = (char *) malloc(strlen(liveUrl) + 1);
    strcpy(this->liveUrl, liveUrl);
    pPacketQueue = new CTPacketQueue();
}

CTLivePush::~CTLivePush() {
    release();
}

void *initConnectFun(void *context) {
    CTLivePush *pLivePush = static_cast<CTLivePush *>(context);
    //1.创建RTMP
    pLivePush->rtmp = RTMP_Alloc();
    //2.初始化
    RTMP_Init(pLivePush->rtmp);
    //3.设置参数
    pLivePush->rtmp->Link.timeout = 10;
    pLivePush->rtmp->Link.lFlags |= RTMP_LF_LIVE;
    RTMP_SetupURL(pLivePush->rtmp, pLivePush->liveUrl);
    RTMP_EnableWrite(pLivePush->rtmp);
    //4.开始连接
    if (!RTMP_Connect(pLivePush->rtmp, NULL)) {
        LOGE("rtmp connect error");
        pLivePush->pJniCall->callConnectError(THREAD_CHILD, INIT_RTMP_CONNECT_ERROR_CODE,
                                              "rtmp connect error");
        return reinterpret_cast<void *>(INIT_RTMP_CONNECT_ERROR_CODE);

    }
    if (!RTMP_ConnectStream(pLivePush->rtmp, 0)) {
        LOGE("rtmp connect stream error");
        pLivePush->pJniCall->callConnectError(THREAD_CHILD, INIT_RTMP_CONNECT_STREAM_ERROR_CODE,
                                              "rtmp connect stream error");
        return reinterpret_cast<void *>(INIT_RTMP_CONNECT_STREAM_ERROR_CODE);
    }
    LOGE("RTMP_Connect success");
    pLivePush->pJniCall->callConnectSuccess(THREAD_CHILD);
    return 0;
}

void CTLivePush::initConnect() {
    pthread_t initConnectTid;
    pthread_create(&initConnectTid, NULL, initConnectFun, this);
    pthread_detach(initConnectTid);
}

void CTLivePush::release() {
    if (rtmp != NULL) {
        RTMP_Close(rtmp);
        free(rtmp);
        rtmp = NULL;
    }
    if (liveUrl != NULL) {
        free(this->liveUrl);
        liveUrl = NULL;
    }
    if (pPacketQueue != NULL) {
        delete pPacketQueue;
        pPacketQueue = NULL;
    }
}
