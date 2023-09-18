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

    pLivePush->startTime = RTMP_GetTime();

    while (pLivePush->isPushing) {
        //不断往流媒体服务器上推
        RTMPPacket *pPacket = pLivePush->pPacketQueue->pop();
        if (pPacket != NULL) {
            int res = RTMP_SendPacket(pLivePush->rtmp, pPacket, 1);
            LOGE("res = %d", res);
            RTMPPacket_Free(pPacket);
            free(pPacket);
        }
    }
    LOGE("livePush停止了");
    return 0;
}

void CTLivePush::initConnect() {

    pthread_create(&initConnectTid, NULL, initConnectFun, this);

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

void CTLivePush::pushSpsPps(jbyte *spsData, jint spsLength, jbyte *ppsData, jint ppsLength) {
    // frame type : 1关键帧，2 非关键帧 (4bit)
    // CodecID : 7表示 AVC (4bit)  , 与 frame type 组合起来刚好是 1 个字节  0x17
    // fixed : 0x00 0x00 0x00 0x00 (4byte)
    // configurationVersion  (1byte)  0x01版本
    // AVCProfileIndication  (1byte)  sps[1] profile
    // profile_compatibility (1byte)  sps[2] compatibility
    // AVCLevelIndication    (1byte)  sps[3] Profile level
    // lengthSizeMinusOne    (1byte)  0xff   包长数据所使用的字节数

    // sps + pps 的数据
    // sps number            (1byte)  0xe1   sps 个数
    // sps data length       (2byte)  sps 长度
    // sps data                       sps 的内容
    // pps number            (1byte)  0x01   pps 个数
    // pps data length       (2byte)  pps 长度
    // pps data                       pps 的内容

    //数据大小 = sps大小 + pps大小 + 额外头信息(16byte)
    int bodySize = spsLength + ppsLength + 16;
    RTMPPacket *pPacket = (RTMPPacket *) malloc(sizeof(RTMPPacket));
    RTMPPacket_Alloc(pPacket, bodySize);
    RTMPPacket_Reset(pPacket);

    //构建body
    char *body = pPacket->m_body;
    int index = 0;
    // CodecID : 7表示 AVC (4bit)  , 与 frame type 组合起来刚好是 1 个字节  0x17
    body[index++] = 0x17;
    // fixed : 0x00 0x00 0x00 0x00 (4byte)
    body[index++] = 0x00;
    body[index++] = 0x00;
    body[index++] = 0x00;
    body[index++] = 0x00;
    // configurationVersion  (1byte)  0x01版本
    body[index++] = 0x01;
    // AVCProfileIndication  (1byte)  sps[1] profile
    body[index++] = spsData[1];
    // profile_compatibility (1byte)  sps[2] compatibility
    body[index++] = spsData[2];
    // AVCLevelIndication    (1byte)  sps[3] Profile level
    body[index++] = spsData[3];
    // lengthSizeMinusOne    (1byte)  0xff   包长数据所使用的字节数
    body[index++] = 0xff;

    // sps + pps 的数据
    // sps number            (1byte)  0xe1   sps 个数
    body[index++] = 0xe1;
    // sps data length       (2byte)  sps 长度
    body[index++] = (spsLength >> 8) & 0xff;  //高8位
    body[index++] = (spsLength) & 0xff;       //低8位
    // sps data                       sps 的内容
    memcpy(&body[index], spsData, spsLength);
    index += spsLength;
    // pps number            (1byte)  0x01   pps 个数
    body[index++] = 0x01;
    // pps data length       (2byte)  pps 长度
    body[index++] = (ppsLength >> 8) & 0xff;  //高8位
    body[index++] = (ppsLength) & 0xff;       //低8位
    // pps data                       pps 的内容
    memcpy(&body[index], ppsData, ppsLength);
    index += ppsLength;

    //设置一些信息
    pPacket->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    pPacket->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    pPacket->m_hasAbsTimestamp = 0;
    pPacket->m_nTimeStamp = 0;
    pPacket->m_nBodySize = bodySize;
    pPacket->m_nChannel = 0x04;
    pPacket->m_nInfoField2 = this->rtmp->m_stream_id;
    pPacketQueue->push(pPacket);
}

void CTLivePush::pushVideo(jbyte *videoData, jint dataLen, jboolean keyFrame) {
    // frame type : 1关键帧，2 非关键帧 (4bit)
    // CodecID : 7表示 AVC (4bit)  , 与 frame type 组合起来刚好是 1 个字节  0x17
    // fixed : 0x01 0x00 0x00 0x00 (4byte)  0x01  表示 NALU 单元

    // video data length       (4byte)  video 长度
    // video data

    int bodySize = dataLen + 9;
    RTMPPacket *pPacket = (RTMPPacket *) malloc(sizeof(RTMPPacket));
    RTMPPacket_Alloc(pPacket, bodySize);
    RTMPPacket_Reset(pPacket);

    //构建body
    char *body = pPacket->m_body;
    int index = 0;

    // frame type : 1关键帧，2 非关键帧 (4bit)
    // CodecID : 7表示 AVC (4bit)  , 与 frame type 组合起来刚好是 1 个字节  0x17
    if (keyFrame) {
        body[index++] = 0x17;
    } else {
        body[index++] = 0x27;
    }

    // fixed : 0x01 0x00 0x00 0x00 (4byte)  0x01  表示 NALU 单元
    body[index++] = 0x01;
    body[index++] = 0x00;
    body[index++] = 0x00;
    body[index++] = 0x00;

    // video data length       (4byte)  video 长度
    body[index++] = (dataLen >> 24) & 0xff;  //高8位
    body[index++] = (dataLen >> 16) & 0xff;  //高8位
    body[index++] = (dataLen >> 8) & 0xff;  //高8位
    body[index++] = (dataLen) & 0xff;       //低8位
    // video data
    memcpy(&body[index], videoData, dataLen);

    //设置一些信息
    pPacket->m_headerType = RTMP_PACKET_SIZE_LARGE;
    pPacket->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    pPacket->m_hasAbsTimestamp = 0;
    pPacket->m_nTimeStamp = RTMP_GetTime() - startTime;
    pPacket->m_nBodySize = bodySize;
    pPacket->m_nChannel = 0x04;
    pPacket->m_nInfoField2 = this->rtmp->m_stream_id;
    pPacketQueue->push(pPacket);
}

void CTLivePush::pushAudio(jbyte *audioData, jint dataLen) {
    // 2 字节头信息
    // 前四位表示音频数据格式 AAC  10(A)
    // 五六位表示采样率 0 = 5.5k  1 = 11k  2 = 22k  3(11) = 44k
    // 七位表示采样采样的精度 0 = 8bits  1 = 16bits
    // 八位表示音频类型  0 = mono  1 = stereo
    // 我们这里算出来第一个字节是 0xAF
    // 0x01 代表 aac 原始数据

    int bodySize = dataLen + 2;
    RTMPPacket *pPacket = (RTMPPacket *) malloc(sizeof(RTMPPacket));
    RTMPPacket_Alloc(pPacket, bodySize);
    RTMPPacket_Reset(pPacket);

    //构建body
    char *body = pPacket->m_body;
    int index = 0;
    // 我们这里算出来第一个字节是 0xAF
    body[index++] = 0xAF;
    // 0x01 代表 aac 原始数据
    body[index++] = 0x01;

    memcpy(&body[index], audioData, dataLen);

    //设置一些信息
    pPacket->m_headerType = RTMP_PACKET_SIZE_LARGE;
    pPacket->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    pPacket->m_hasAbsTimestamp = 0;
    pPacket->m_nTimeStamp = RTMP_GetTime() - startTime;
    pPacket->m_nBodySize = bodySize;
    pPacket->m_nChannel = 0x04;
    pPacket->m_nInfoField2 = this->rtmp->m_stream_id;
    pPacketQueue->push(pPacket);
}

void CTLivePush::stop() {
    isPushing = false;
    pPacketQueue->notify();
    pthread_join(initConnectTid, NULL);
    LOGE("等待停止了");
}

