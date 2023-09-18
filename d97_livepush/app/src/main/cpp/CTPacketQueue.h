//
// Created by cnting on 2023/9/8.
//

#ifndef D3_FFMPEG_CTPACKETQUEUE_H
#define D3_FFMPEG_CTPACKETQUEUE_H

#include <queue>
#include "pthread.h"

extern "C" {
#include "rtmp.h"
};

class CTPacketQueue {
public:
    std::queue<RTMPPacket *> *pPacketQueue;
    pthread_mutex_t packetMutex;
    pthread_cond_t packetCond;
public:
    CTPacketQueue();

    ~CTPacketQueue();

    void push(RTMPPacket *pPacket);

    RTMPPacket *pop();

    void clear();

    void notify();
};


#endif //D3_FFMPEG_CTPACKETQUEUE_H
