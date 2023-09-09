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

    pPlayerStatus = new CTPlayerStatus();
}

CTFFmpeg::~CTFFmpeg() {
    release();
}

void *threadDecodePacket(void *context) {
    CTFFmpeg *pFFempeg = (CTFFmpeg *) context;

    while (pFFempeg->pPlayerStatus != NULL && !pFFempeg->pPlayerStatus->isExit) {
        AVPacket *pPacket = av_packet_alloc();
        if (av_read_frame(pFFempeg->pFormatContext, pPacket) >= 0) {
            if (pPacket->stream_index == pFFempeg->pAudio->streamIndex) {
                pFFempeg->pAudio->pPacketQueue->push(pPacket);
            } else if (pPacket->stream_index == pFFempeg->pVideo->streamIndex) {
                pFFempeg->pVideo->pPacketQueue->push(pPacket);
            } else {
                av_packet_free(&pPacket);
            }
        } else {
            av_packet_free(&pPacket);
//            break;
        }
    }
    return 0;
}

void CTFFmpeg::play() {
    pthread_t readPacketThreadT;
    pthread_create(&readPacketThreadT, NULL, threadDecodePacket, this);
    pthread_detach(readPacketThreadT);


    if (pAudio != NULL) {
        pAudio->play();
    }
    if (pVideo != NULL) {
        pVideo->play();
    }
}

void CTFFmpeg::callPlayJniError(ThreadMode threadMode, int code, char *msg) {
    release();
    pJniCall->callPlayerError(threadMode, code, msg);
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

    pAudio = new CTAudio(audioStreamIndex, pJniCall, pPlayerStatus);
    pAudio->analysisStream(threadMode, pFormatContext);

    //查找视频流的index
    int videoStreamIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_VIDEO, -1,
                                               -1,
                                               NULL, 0);
    if (videoStreamIndex < 0) {
        LOGE("find video stream error");
        callPlayJniError(threadMode, FIND_STREAM_ERROR_CODE, "find video stream error");
        return;
    }
    pVideo = new CTVideo(videoStreamIndex, pJniCall, pPlayerStatus);
    pVideo->analysisStream(threadMode, pFormatContext);

    pJniCall->callPlayPrepared(threadMode);
}

void CTFFmpeg::setSurface(jobject surface) {
    if (pVideo != NULL) {
        pVideo->setSurface(surface);
    }
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
    if (pPlayerStatus != NULL) {
        delete (pPlayerStatus);
        pPlayerStatus = NULL;
    }
    if (pVideo != NULL) {
        delete pVideo;
        pVideo = NULL;
    }
    if (pAudio != NULL) {
        delete pAudio;
        pAudio = NULL;
    }
}






