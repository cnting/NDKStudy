//
// Created by cnting on 2023/9/8.
//

#include "CTVideo.h"
#include "constants.h"


CTVideo::CTVideo(int videoStreamIndex, CTJNICall *pJniCall, CTPlayerStatus *pPlayStatus,
                 CTAudio *pAudio) : CTMedia(
        videoStreamIndex, pJniCall, pPlayStatus) {
    this->pAudio = pAudio;
}

CTVideo::~CTVideo() {
    release();
}

void *threadVideoPlay(void *context) {
    CTVideo *pVideo = (CTVideo *) context;

    //当前是子线程
    JNIEnv *env;
    if (pVideo->pJniCall->javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
        LOGE("get child thread jniEnv error!");
        return 0;
    }

    AVPacket *pPacket = NULL;
    AVFrame *pFrame = av_frame_alloc();
    ANativeWindow_Buffer outBuffer;

    //1.获取窗体
    ANativeWindow *pNativeWindow = ANativeWindow_fromSurface(env, pVideo->surface);
    pVideo->pJniCall->javaVM->DetachCurrentThread();
    //2.设置缓冲区属性
    ANativeWindow_setBuffersGeometry(pNativeWindow, pVideo->pCodecContext->width,
                                     pVideo->pCodecContext->height,
                                     WINDOW_FORMAT_RGBA_8888);

    //读取的packet是压缩数据
    while (pVideo->pPlayerStatus != NULL && !pVideo->pPlayerStatus->isExit) {
        pPacket = pVideo->pPacketQueue->pop();
        //packet放到解码队列中去解码
        int codeSendPacketRes = avcodec_send_packet(pVideo->pCodecContext, pPacket);
        if (codeSendPacketRes == 0) {
            //从成功的解码队列中取出一帧pcm数据
            int codecReceiveFrameRes = avcodec_receive_frame(pVideo->pCodecContext, pFrame);
            if (codecReceiveFrameRes == 0) {
                //pFrame->data 一般都是yuv420p，但要显示RGBA8888，因此需要转换
                sws_scale(pVideo->pSwsContext, pFrame->data, pFrame->linesize,
                          0, pVideo->pCodecContext->height,
                          pVideo->pRgbaFrame->data, pVideo->pRgbaFrame->linesize);

                //判断需要休眠多久
                double frameSleepTime = pVideo->getFrameSleepTime(pFrame);
                av_usleep(frameSleepTime * 1000000);

                //把数据推到缓冲区
                ANativeWindow_lock(pNativeWindow, &outBuffer, NULL);
                memcpy(outBuffer.bits, pVideo->pFrameBuffer, pVideo->frameSize);
                ANativeWindow_unlockAndPost(pNativeWindow);
            }
        }
        //解引用
        av_packet_unref(pPacket);
        av_frame_unref(pFrame);
    }
    //1.解引用 data  2.销毁pPacket结构体内存  3.pPacket置为空
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);


    return 0;
}

void CTVideo::play() {
    pthread_t playThreadT;
    pthread_create(&playThreadT, NULL, threadVideoPlay, this);
    pthread_detach(playThreadT);
}

void CTVideo::privateAnalysisStream(ThreadMode threadMode, AVFormatContext *pFormatContext) {
    pSwsContext = sws_getContext(pCodecContext->width, pCodecContext->height,
                                 pCodecContext->pix_fmt,
                                 pCodecContext->width, pCodecContext->height,
                                 AV_PIX_FMT_RGBA,
                                  NULL, NULL, NULL, NULL
    );
    pRgbaFrame = av_frame_alloc();
    frameSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, pCodecContext->width,
                                         pCodecContext->height, 1);
    pFrameBuffer = static_cast<uint8_t *>(malloc(frameSize * sizeof(uint8_t)));
    av_image_fill_arrays(pRgbaFrame->data, pRgbaFrame->linesize,
                         pFrameBuffer, AV_PIX_FMT_RGBA,
                         pCodecContext->width, pCodecContext->height, 1);

    int num = pFormatContext->streams[streamIndex]->avg_frame_rate.num;
    int den = pFormatContext->streams[streamIndex]->avg_frame_rate.den;
    if (den != 0 && num != 0) {
        defaultDelayTime = 1.0f * den / num;
    }
}

void CTVideo::release() {
    CTMedia::release();
    if (pSwsContext != NULL) {
        sws_freeContext(pSwsContext);
        free(pSwsContext);
        pSwsContext = NULL;
    }
    if (pFrameBuffer != NULL) {
        free(pFrameBuffer);
        pFrameBuffer = NULL;
    }
    if (pRgbaFrame != NULL) {
        av_frame_free(&pRgbaFrame);
        pRgbaFrame = NULL;
    }
    if (pJniCall != NULL) {
        pJniCall->jniEnv->DeleteGlobalRef(surface);
    }
}

void CTVideo::setSurface(jobject surface) {
    this->surface = pJniCall->jniEnv->NewGlobalRef(surface);
}

double CTVideo::getFrameSleepTime(AVFrame *pFrame) {
    double times = pFrame->pts * av_q2d(timeBase);
    if (times > currentTime) {
        currentTime = times;
    }
    double diffTime = pAudio->currentTime - currentTime;
    //视频快了就慢点，慢了就快点，但要把时间控制在视频的帧率时间范围左右
    //控制在1秒60帧
    if (diffTime > 0.016 || diffTime < -0.016) {
        //视频慢了
        if (diffTime > 0.016) {
            delayTime = delayTime * 2 / 3;
        } else if (diffTime < -0.016) {
            delayTime = delayTime * 3 / 2;
        }

        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime * 2 / 3;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 3 / 2;
        }
    }
    if (diffTime >= 0.25) {
        delayTime = 0;
    } else if (diffTime <= -0.25) {
        delayTime = defaultDelayTime * 2;
    }
    return delayTime;
}
