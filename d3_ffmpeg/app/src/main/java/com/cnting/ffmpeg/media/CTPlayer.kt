package com.cnting.ffmpeg.media

import com.cnting.ffmpeg.media.listener.MediaErrorListener
import com.cnting.ffmpeg.media.listener.MediaInfoListener
import com.cnting.ffmpeg.media.listener.MediaPreparedListener

/**
 * Created by cnting on 2023/9/6
 *
 */
class CTPlayer {

    companion object {
        init {
            System.loadLibrary("music-player")
        }
    }

    private var url: String? = null
    var errorListener: MediaErrorListener? = null
    var preparedListener: MediaPreparedListener? = null
    var mediaInfoListener: MediaInfoListener? = null

    /**
     * called from jni
     */
    private fun onError(code: Int, msg: String) {
        errorListener?.onError(code, msg)
    }

    /**
     * called from jni
     */
    private fun onPrepared() {
        preparedListener?.onPrepared()
    }

    /**
     * called from jni
     */
    private fun musicInfo(sampleRate: Int, channels: Int) {
        mediaInfoListener?.musicInfo(sampleRate, channels)
    }

    /**
     * called from jni
     */
    private fun callbackPcm(pcmData: ByteArray, size: Int) {
        mediaInfoListener?.callbackPcm(pcmData, size)
    }

    fun setDataSource(url: String) {
        this.url = url;
    }

    fun play() {
        nPlay()
    }

    fun stop() {
        nStop()
    }

    fun release() {
        nRelease()
    }

    fun prepare() {
        val finalUrl = url ?: throw NullPointerException("url is null! please call setDataSource()")
        nPrepare(finalUrl)
    }

    fun prepareAsync() {
        val finalUrl = url ?: throw NullPointerException("url is null! please call setDataSource()")
        nPrepareAsync(finalUrl)
    }

    private external fun nPlay()
    private external fun nStop()
    private external fun nRelease()
    private external fun nPrepare(url: String)
    private external fun nPrepareAsync(url: String)

}