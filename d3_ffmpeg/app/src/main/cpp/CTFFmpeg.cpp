//
// Created by cnting on 2023/9/7.
//

#include "CTFFmpeg.h"
#include "constants.h"
#include "pthread.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

CTFFmpeg::CTFFmpeg(CTJNICall *pJniCall, const char *url) {
    this->pJniCall = pJniCall;
    //复制一份，避免外面销毁
    this->url = (char *) malloc(strlen(url) + 1);
    memcpy(this->url, url, strlen(url) + 1);
}

CTFFmpeg::~CTFFmpeg() {
    release();
}


void CTFFmpeg::play() {
    if (pAudio != NULL) {
        pAudio->play();
    }
}

void CTFFmpeg::callPlayJniError(ThreadMode threadMode, int code, char *msg) {
    release();
    pJniCall->callPlayerError(threadMode, code, msg);
}

void CTFFmpeg::release() {

    if (pFormatContext != NULL) {
        //释放流的资源
        avformat_close_input(&pFormatContext);
        //释放开辟的AVFormatContext结构体内存
        avformat_free_context(pFormatContext);
        pFormatContext = NULL;
    }

    avformat_network_deinit();
    if (url != NULL) {
        free(this->url);
        url = NULL;
    }
}


void CTFFmpeg::prepare() { prepare(THREAD_MAIN); }

void *threadPrepare(void *context) {
    CTFFmpeg *pFFmpeg = (CTFFmpeg *) context;
    pFFmpeg->prepare(THREAD_CHILD);
    return 0;
}

void CTFFmpeg::preparedAsync() {
    pthread_t prepareThreadT;
    pthread_create(&prepareThreadT, NULL, threadPrepare, this);
    pthread_detach(prepareThreadT);
}

void CTFFmpeg::prepare(ThreadMode threadMode) {
    avformat_network_init();

    //读取文件头，保存到pFormatContext
    int formatOpenInputRes = avformat_open_input(&pFormatContext, url, NULL, NULL);
    if (formatOpenInputRes != 0) {
        LOGE("format open input error:%s", av_err2str(formatOpenInputRes));
        callPlayJniError(threadMode, formatOpenInputRes, av_err2str(formatOpenInputRes));
        return;
    }

    //读取一部分视音频数据并获取一些相关信息
    int formatFindStreamInfo = avformat_find_stream_info(pFormatContext, NULL);
    if (formatFindStreamInfo < 0) {
        LOGE("format open input error:%s", av_err2str(formatFindStreamInfo));
        callPlayJniError(threadMode, formatFindStreamInfo, av_err2str(formatFindStreamInfo));
        return;
    }

    //查找音频流的index
    int audioStreamIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_AUDIO, -1,
                                               -1,
                                               NULL, 0);
    if (audioStreamIndex < 0) {
        LOGE("find audio stream error");
        callPlayJniError(threadMode, FIND_STREAM_ERROR_CODE, "find audio stream error");
        return;
    }

    pAudio = new CTAudio(audioStreamIndex, pJniCall, pFormatContext);
    pAudio->analysisStream(threadMode);

    pJniCall->callPlayPrepared(threadMode);
}






