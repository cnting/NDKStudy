package com.cnting.ffmpeg.media

import com.cnting.ffmpeg.media.listener.MediaErrorListener
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

    fun setDataSource(url: String) {
        this.url = url;
    }

    fun play() {
        nPlay()
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
    private external fun nRelease()
    private external fun nPrepare(url: String)
    private external fun nPrepareAsync(url: String)

}