//
// Created by cnting on 2023/9/7.
//

#include "CTFFmpeg.h"
#include "constants.h"
#include "pthread.h"

CTFFmpeg::CTFFmpeg(CTJNICall *pJniCall, const char *url) {
    this->pJniCall = pJniCall;
    //复制一份，避免外面销毁
    this->url = (char *) malloc(strlen(url) + 1);
    memcpy(this->url, url, strlen(url) + 1);
}

CTFFmpeg::~CTFFmpeg() {
    release();
}

void *threadPlay(void *context) {
    CTFFmpeg *pFFmpeg = (CTFFmpeg *) context;
    pFFmpeg->prepare(THREAD_CHILD);
    return 0;
}

void CTFFmpeg::play() {
    //在子线程播
    pthread_t playThreadT;
    pthread_create(&playThreadT, NULL, threadPlay, this);
    pthread_detach(playThreadT);
}

void CTFFmpeg::callPlayJniError(ThreadMode threadMode, int code, char *msg) {
    release();
    pJniCall->callPlayerError(threadMode, code, msg);
}

void CTFFmpeg::release() {
    if (pCodecContext != NULL) {
        avcodec_close(pCodecContext);
        avcodec_free_context(&pCodecContext);
        pCodecContext = NULL;
    }
    if (pFormatContext != NULL) {
        //释放流的资源
        avformat_close_input(&pFormatContext);
        //释放开辟的AVFormatContext结构体内存
        avformat_free_context(pFormatContext);
        pFormatContext = NULL;
    }
    if (swrContext != NULL) {
        swr_free(&swrContext);
        free(swrContext);
        swrContext = NULL;
    }
    if (resampleOutBuffer != NULL) {
        free(resampleOutBuffer);
        resampleOutBuffer = NULL;
    }
    avformat_network_deinit();
    if (url != NULL) {
        free(this->url);
        url = NULL;
    }
}

void CTFFmpeg::prepare() { prepare(THREAD_MAIN); }

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

    //查找解码器
    AVCodecParameters *pCodecParameters = pFormatContext->streams[audioStreamIndex]->codecpar;
    const AVCodec *pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    if (pCodec == NULL) {
        LOGE("find decoder error");
        callPlayJniError(threadMode, CODEC_FIND_DECODER_ERROR_CODE, "find decoder error");
        return;
    }

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

    //-------------重采样 start------------
    const AVChannelLayout out_ch_layout = AV_CHANNEL_LAYOUT_STEREO;
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_sample_rate = AUDIO_SAMPLE_RATE;
    const AVChannelLayout in_ch_layout = pCodecContext->ch_layout;
    enum AVSampleFormat in_sample_fmt = pCodecContext->sample_fmt;
    int in_sample_rate = pCodecContext->sample_rate;
    int log_offset = 0;
    void *log_ctx = NULL;

    LOGE("in===>channel:%d,format:%d,rate:%d", in_ch_layout.nb_channels, in_sample_fmt,
         in_sample_rate);
    LOGE("out===>channel:%d,format:%d,rate:%d", out_ch_layout.nb_channels, out_sample_fmt,
         out_sample_rate);

    swr_alloc_set_opts2(&swrContext, &out_ch_layout, out_sample_fmt, out_sample_rate,
                        &in_ch_layout, in_sample_fmt, in_sample_rate,
                        log_offset, log_ctx);
    if (swrContext == NULL) {
        callPlayJniError(threadMode, SWR_ALLOC_SET_OPTS_ERROR_CODE, "swr alloc set opts error");
        return;
    }
    int swrInitRes = swr_init(swrContext);
    if (swrInitRes < 0) {
        callPlayJniError(threadMode, SWR_CONTEXT_INIT_ERROR_CODE, "swr init error");
        return;
    }

    //写到AudioTrack.write(@NonNull byte[] audioData, int offsetInBytes, int sizeInBytes)
    //要把pFrame.data转成java byte
    int dataSize = av_samples_get_buffer_size(NULL,
                                              out_ch_layout.nb_channels,
                                              pCodecContext->frame_size,
                                              out_sample_fmt,
                                              0);
    resampleOutBuffer = static_cast<uint8_t *>(malloc(dataSize));
    //-------------重采样 end------------


    //创建一个Java类型数组
    jbyteArray jPcmByteArray = pJniCall->jniEnv->NewByteArray(dataSize);
    //把Java类型数组转成C类型数组
    jbyte *jPcmData = pJniCall->jniEnv->GetByteArrayElements(jPcmByteArray, 0);

    AVPacket *pPacket = av_packet_alloc();
    AVFrame *pFrame = av_frame_alloc();
    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        if (pPacket->stream_index == audioStreamIndex) {
            //发送数据到ffmpeg，放到解码队列中
            int codeSendPacketRes = avcodec_send_packet(pCodecContext, pPacket);
            if (codeSendPacketRes == 0) {
                //从成功的解码队列中取出一帧
                int codecReceiveFrameRes = avcodec_receive_frame(pCodecContext, pFrame);
                if (codecReceiveFrameRes == 0) {
                    if (pCodecContext->frame_size != pFrame->nb_samples) {
                        LOGE("上面设置的:%d,pFrame->nb_samples:%d", pCodecContext->frame_size,
                             pFrame->nb_samples);
                    }
                    //重采样
                    swr_convert(swrContext, &resampleOutBuffer, pCodecContext->frame_size,
                                (const uint8_t **) pFrame->data, pFrame->nb_samples);


                    memcpy(jPcmData, resampleOutBuffer, dataSize);
                    pJniCall->jniEnv->
                            ReleaseByteArrayElements(jPcmByteArray, jPcmData, JNI_COMMIT);
                    pJniCall->
                            callAudioTrackWrite(jPcmByteArray,
                                                0, dataSize);
                }
            }
        }
        //解引用
        av_packet_unref(pPacket);
        av_frame_unref(pFrame);
    }
    //1.解引用 data  2.销毁pPacket结构体内存  3.pPacket置为空
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    pJniCall->jniEnv->
            ReleaseByteArrayElements(jPcmByteArray, jPcmData,
                                     0);
    pJniCall->jniEnv->
            DeleteLocalRef(jPcmByteArray);
}

void CTFFmpeg::preparedAsync() {

}


