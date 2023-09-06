#include <jni.h>
#include <string>
#include "constants.h"
//在c++中采用c的这种编译方式
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}


jobject initCreateAudioTrack(JNIEnv *env) {
/**
 * public AudioTrack(int streamType, int sampleRateInHz, int channelConfig, int audioFormat,
            int bufferSizeInBytes, int mode)
 */
    //创建AudioTrack
    int streamType = 3;
    int sampleRateInHz = AUDIO_SAMPLE_RATE;
    int channelConfig = (0x4 | 0x8);
    int audioFormat = 2;
    int bufferSizeInBytes;
    int mode = 1;

    jclass jAudioTrackClass = env->FindClass("android/media/AudioTrack");
    jmethodID jAudioTrackMid = env->GetMethodID(jAudioTrackClass, "<init>", "(IIIIII)V");

    jmethodID getMinBufferSizeMid = env->GetStaticMethodID(jAudioTrackClass, "getMinBufferSize",
                                                           "(III)I");
    bufferSizeInBytes = env->CallStaticIntMethod(jAudioTrackClass, getMinBufferSizeMid,
                                                 sampleRateInHz, channelConfig, audioFormat);

    jobject audioTrack = env->NewObject(jAudioTrackClass, jAudioTrackMid, streamType,
                                        sampleRateInHz, channelConfig,
                                        audioFormat, bufferSizeInBytes, mode);

    //调用play()
    jmethodID playMid = env->GetMethodID(jAudioTrackClass, "play", "()V");
    env->CallVoidMethod(audioTrack, playMid);

    return audioTrack;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_ffmpeg_media_CTPlayer_nPlay(JNIEnv *env, jobject thiz, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    avformat_network_init();

    AVFormatContext *pFormatContext = NULL;
    int formatOpenInputRes = 0;
    int formatFindStreamInfo = 0;
    int audioStreamIndex = -1;
    AVCodecParameters *pCodecParameters = NULL;
    const AVCodec *pCodec = NULL;
    AVCodecContext *pCoderContext = NULL;
    int codecParametersToContextRes = 0;
    int codecOpenRes = -1;
    AVPacket *pPacket = NULL;
    AVFrame *pFrame = NULL;
    int index = 0;
    jobject jAudioTrackObj;
    jclass jAudioTrackClass;
    jmethodID jWriteMid;

    //读取文件头，保存到pFormatContext
    formatOpenInputRes = avformat_open_input(&pFormatContext, url, NULL, NULL);
    if (formatOpenInputRes != 0) {
        //需要回调给Java层
        //需要释放资源

        LOGE("format open input error:%s", av_err2str(formatOpenInputRes));
        goto __av_resources_destroy;
    }

    //读取一部分视音频数据并获取一些相关信息
    formatFindStreamInfo = avformat_find_stream_info(pFormatContext, NULL);
    if (formatFindStreamInfo < 0) {
        LOGE("format open input error");
        goto __av_resources_destroy;
    }

    //查找音频流的index
    audioStreamIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_AUDIO, -1, -1,
                                           NULL, 0);
    if (audioStreamIndex < 0) {
        LOGE("find best stream error");
        goto __av_resources_destroy;
    }

    //查找解码器
    pCodecParameters = pFormatContext->streams[audioStreamIndex]->codecpar;
    pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    if (pCodec == NULL) {
        LOGE("find decoder error");
        goto __av_resources_destroy;
    }


    pCoderContext = avcodec_alloc_context3(pCodec);
    if (pCoderContext == NULL) {
        LOGE("codec alloc context error");
        goto __av_resources_destroy;
    }
    //把 pCodecParameters 数据放到 pCoderContext 中
    codecParametersToContextRes = avcodec_parameters_to_context(pCoderContext, pCodecParameters);
    if (codecParametersToContextRes < 0) {
        LOGE("codec parameters to context error:%s", av_err2str(codecParametersToContextRes));
        goto __av_resources_destroy;
    }
    //打开解码器
    codecOpenRes = avcodec_open2(pCoderContext, pCodec, NULL);
    if (codecOpenRes != 0) {
        LOGE("codec audio open error:%s", av_err2str(codecOpenRes));
        goto __av_resources_destroy;
    }

    jAudioTrackClass = env->FindClass("android/media/AudioTrack");
    jWriteMid = env->GetMethodID(jAudioTrackClass, "write", "([BII)I");
    jAudioTrackObj = initCreateAudioTrack(env);
    pPacket = av_packet_alloc();
    pFrame = av_frame_alloc();
    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        if (pPacket->stream_index == audioStreamIndex) {
            //发送数据到ffmpeg，放到解码队列中
            int codeSendPacketRes = avcodec_send_packet(pCoderContext, pPacket);
            if (codeSendPacketRes == 0) {
                //从成功的解码队列中取出一帧
                int codecReceiveFrameRes = avcodec_receive_frame(pCoderContext, pFrame);
                if (codecReceiveFrameRes == 0) {
                    //AVPacket 解码成了 AVFrame
                    index++;
                    LOGE("解码第%d帧", index);

                    //写到AudioTrack.write(@NonNull byte[] audioData, int offsetInBytes, int sizeInBytes)
                    //要把pFrame.data转成java byte
                    //1秒有44100个点，如果是2通道，2字节，1秒长度=44100*2*2
                    int dataSize = av_samples_get_buffer_size(NULL,
                                                              pFrame->ch_layout.nb_channels,
                                                              pFrame->nb_samples,
                                                              pCoderContext->sample_fmt,
                                                              0);
                    //创建一个Java类型数组
                    jbyteArray jPcmByteArray = env->NewByteArray(dataSize);
                    //把Java类型数组转成C类型数组
                    jbyte *jPcmData = env->GetByteArrayElements(jPcmByteArray, 0);
                    memcpy(jPcmData, pFrame->data, dataSize);
                    //0: jbyteArray 同步到java中，并释放native数组
                    env->ReleaseByteArrayElements(jPcmByteArray, jPcmData, 0);

                    env->CallIntMethod(jAudioTrackObj, jWriteMid, jPcmByteArray, 0, dataSize);
                    env->DeleteLocalRef(jPcmByteArray);
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
    env->DeleteLocalRef(jAudioTrackObj);

    __av_resources_destroy:
    if (pCoderContext != NULL) {
        avcodec_close(pCoderContext);
        avcodec_free_context(&pCoderContext);
        pCoderContext = NULL;
    }
    if (pFormatContext != NULL) {
        //释放流的资源
        avformat_close_input(&pFormatContext);
        //释放开辟的AVFormatContext结构体内存
        avformat_free_context(pFormatContext);
        pFormatContext = NULL;
    }
    avformat_network_deinit();
    env->ReleaseStringUTFChars(url_, url);
}