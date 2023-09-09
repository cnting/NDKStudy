//
// Created by cnting on 2023/9/8.
//

#include "CTPacketQueue.h"

CTPacketQueue::CTPacketQueue() {
    pPacketQueue = new std::queue<AVPacket *>();
    pthread_mutex_init(&packetMutex, NULL);
    pthread_cond_init(&packetCond, NULL);
}

CTPacketQueue::~CTPacketQueue() {
    if (pPacketQueue != NULL) {
        clear();
        delete (pPacketQueue);
        pPacketQueue = NULL;
    }
    pthread_mutex_destroy(&packetMutex);
    pthread_cond_destroy(&packetCond);
}

void CTPacketQueue::push(AVPacket *pPacket) {
    pthread_mutex_lock(&packetMutex);
    pPacketQueue->push(pPacket);
    pthread_cond_signal(&packetCond);
    pthread_mutex_unlock(&packetMutex);
}

AVPacket *CTPacketQueue::pop() {
    AVPacket *packet;
    pthread_mutex_lock(&packetMutex);
    while (pPacketQueue->empty()) {
        pthread_cond_wait(&packetCond, &packetMutex);
    }
    packet = pPacketQueue->front();
    pPacketQueue->pop();
    pthread_mutex_unlock(&packetMutex);
    return packet;
}

/**
 * 清除队列，还要清除每个AVPacket*的内存数据
 */
void CTPacketQueue::clear() {

}
