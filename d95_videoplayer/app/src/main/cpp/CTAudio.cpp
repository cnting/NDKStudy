//
// Created by cnting on 2023/9/7.
//

#include "CTAudio.h"
#include "pthread.h"
#include "constants.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

CTAudio::CTAudio(int audioStreamIndex, CTJNICall *pJniCall, CTPlayerStatus *pPlayerStatus)
        : CTMedia(audioStreamIndex, pJniCall, pPlayerStatus) {
}

void *threadAudioPlay(void *context) {
    CTAudio *audio = (CTAudio *) context;
    audio->initCreateOpenSLES();
    return 0;
}

void CTAudio::play() {
    pthread_t playThreadT;
    pthread_create(&playThreadT, NULL, threadAudioPlay, this);
    pthread_detach(playThreadT);
}

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    CTAudio *audio = (CTAudio *) context;
    int dataSize = audio->resampleAudio();
    (*bq)->Enqueue(bq, audio->resampleOutBuffer, dataSize);
}

void CTAudio::initCreateOpenSLES() {
    //1. 创建引擎接口对象
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine;

    // create engine
    slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);

    // realize the engine
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);

    // get the engine interface, which is needed in order to create other objects
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);


    //2. 设置混音器
    SLObjectItf outputMixObject = NULL;
    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);

    // realize the output mix
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    // get the environmental reverb interface
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings =
            SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                     &outputMixEnvironmentalReverb);
    (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
            outputMixEnvironmentalReverb, &reverbSettings);


    //3. 创建播放器
    SLObjectItf pPlayer = NULL;
    SLPlayItf bqPlayerPlay;
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource pAudioSrc = {&loc_bufq, &format_pcm};
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink pAudioSnk = {&loc_outmix, NULL};
    const SLInterfaceID pInterfaceIds[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME,
                                            SL_IID_PLAYBACKRATE,};
    const SLboolean pInterfaceRequired[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,};
    (*engineEngine)->CreateAudioPlayer(engineEngine, &pPlayer, &pAudioSrc, &pAudioSnk,
                                       3, pInterfaceIds, pInterfaceRequired);
    // realize the player
    (*pPlayer)->Realize(pPlayer, SL_BOOLEAN_FALSE);

    // get the play interface
    (*pPlayer)->GetInterface(pPlayer, SL_IID_PLAY, &bqPlayerPlay);

    //4. 设置缓存队列和回调函数
    // get the buffer queue interface
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
    (*pPlayer)->GetInterface(pPlayer, SL_IID_BUFFERQUEUE,
                             &bqPlayerBufferQueue);
    // register callback on the buffer queue
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);


    //5. 设置播放状态
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

    //6. 调用回调函数
    bqPlayerCallback(bqPlayerBufferQueue, this);
}

int CTAudio::resampleAudio() {
    int dataSize = 0;
    AVPacket *pPacket = NULL;
    AVFrame *pFrame = av_frame_alloc();
    //读取的packet是压缩数据
    while (pPlayerStatus != NULL && !pPlayerStatus->isExit) {
        pPacket = pPacketQueue->pop();
        //packet放到解码队列中去解码
        int codeSendPacketRes = avcodec_send_packet(pCodecContext, pPacket);
        if (codeSendPacketRes == 0) {
            //从成功的解码队列中取出一帧pcm数据
            int codecReceiveFrameRes = avcodec_receive_frame(pCodecContext, pFrame);
            if (codecReceiveFrameRes == 0) {
                //重采样，返回的是重采样的个数，也就是pFrame->nb_samples
                dataSize = swr_convert(swrContext, &resampleOutBuffer,
                                       pFrame->nb_samples,
                                       (const uint8_t **) pFrame->data,
                                       pFrame->nb_samples);
                dataSize = dataSize * 2 * 2;

                //设置当前时间
                double times = pFrame->pts * av_q2d(timeBase);
                if (times > currentTime) {
                    currentTime = times;
                }

                break;
            }
        }
        //解引用
        av_packet_unref(pPacket);
        av_frame_unref(pFrame);
    }
    //1.解引用 data  2.销毁pPacket结构体内存  3.pPacket置为空
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    return dataSize;
}

CTAudio::~CTAudio() {
    release();
}

void CTAudio::release() {
    CTMedia::release();
    if (resampleOutBuffer != NULL) {
        free(resampleOutBuffer);
        resampleOutBuffer = NULL;
    }

    if (swrContext != NULL) {
        swr_free(&swrContext);
        free(swrContext);
        swrContext = NULL;
    }
}

void CTAudio::privateAnalysisStream(ThreadMode threadMode, AVFormatContext *pFormatContext) {
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

    resampleOutBuffer = static_cast<uint8_t *>(malloc(pCodecContext->frame_size * 2 * 2));
    //-------------重采样 end------------

}

