//
// Created by cnting on 2023/9/9.
//

#include "CTMedia.h"
#include "constants.h"

CTMedia::CTMedia(int streamIndex, CTJNICall *pJniCall, CTPlayerStatus *pPlayerStatus) {
    this->streamIndex = streamIndex;
    this->pJniCall = pJniCall;
    this->pPlayerStatus = pPlayerStatus;
    this->pPacketQueue = new CTPacketQueue();
    this->pPlayerStatus = new CTPlayerStatus();
}

CTMedia::~CTMedia() {
    release();
}

void CTMedia::analysisStream(ThreadMode threadMode, AVFormatContext* pFormatContext) {
    publicAnalysisStream(threadMode, pFormatContext);
    privateAnalysisStream(threadMode, pFormatContext);
}

void CTMedia::release() {
    if (pPacketQueue != NULL) {
        delete pPacketQueue;
        pPacketQueue = NULL;
    }
    if (pPlayerStatus != NULL) {
        delete pPlayerStatus;
        pPlayerStatus = NULL;
    }
    if (pCodecContext != NULL) {
        avcodec_close(pCodecContext);
        avcodec_free_context(&pCodecContext);
        pCodecContext = NULL;
    }
}

void CTMedia::publicAnalysisStream(ThreadMode threadMode,  AVFormatContext* pFormatContext) {
    //查找解码器
    AVCodecParameters *pCodecParameters = pFormatContext->streams[streamIndex]->codecpar;
    const AVCodec *pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    if (pCodec == NULL) {
        LOGE("find decoder error");
        callPlayJniError(threadMode, CODEC_FIND_DECODER_ERROR_CODE, "find decoder error");
        return;
    }
    //分配一个AVCodecContext，记得销毁
    pCodecContext = avcodec_alloc_context3(pCodec);
    if (pCodecContext == NULL) {
        LOGE("codec alloc context error");
        callPlayJniError(threadMode, CODEC_ALLOC_CONTEXT_ERROR_CODE, "codec alloc context error");
        return;
    }
    //把 pCodecParameters 数据放到 pCoderContext 中
    int codecParametersToContextRes = avcodec_parameters_to_context(pCodecContext,
                                                                    pCodecParameters);
    if (codecParametersToContextRes < 0) {
        LOGE("codec parameters to context error:%s", av_err2str(codecParametersToContextRes));
        callPlayJniError(threadMode, codecParametersToContextRes,
                         av_err2str(codecParametersToContextRes));
        return;
    }
    //打开解码器
    int codecOpenRes = avcodec_open2(pCodecContext, pCodec, NULL);
    if (codecOpenRes != 0) {
        LOGE("codec audio open error:%s", av_err2str(codecOpenRes));
        callPlayJniError(threadMode, codecOpenRes, av_err2str(codecOpenRes));
        return;
    }
    duration = pFormatContext->duration;
    timeBase = pFormatContext->streams[streamIndex]->time_base;

    LOGE("timeBase: num:%d,den:%d",timeBase.num,timeBase.num);
}

void CTMedia::callPlayJniError(ThreadMode threadMode, int code, char *msg) {
    release();
    pJniCall->callPlayerError(threadMode, code, msg);
}


