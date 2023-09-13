package com.cnting.ffmpeg.media.listener

/**
 * Created by cnting on 2023/9/12
 *
 */
interface MediaInfoListener {
    fun musicInfo(sampleRate: Int, channels: Int)
    fun callbackPcm(pcmData: ByteArray, size: Int)
}