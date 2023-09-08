//
// Created by cnting on 2023/9/8.
//

#ifndef D3_FFMPEG_CTPACKETQUEUE_H
#define D3_FFMPEG_CTPACKETQUEUE_H

#include <queue>
#include "pthread.h"

extern "C" {
#include "includes/libavformat/avformat.h"
#include "includes/libavcodec/avcodec.h"
#include "includes/libswresample/swresample.h"
};

class CTPacketQueue {
public:
    std::queue<AVPacket *> *pPacketQueue;
    pthread_mutex_t packetMutex;
    pthread_cond_t packetCond;
public:
    CTPacketQueue();

    ~CTPacketQueue();

    void push(AVPacket *pPacket);

    AVPacket *pop();

    void clear();
};


#endif //D3_FFMPEG_CTPACKETQUEUE_H
