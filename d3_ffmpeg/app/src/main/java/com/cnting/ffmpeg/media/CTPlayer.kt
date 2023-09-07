package com.cnting.ffmpeg.media

import com.cnting.ffmpeg.media.listener.MediaErrorListener

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
    private var errorListener: MediaErrorListener? = null

    fun setErrorListener(errorListener: MediaErrorListener) {
        this.errorListener = errorListener
    }

    /**
     * called from jni
     */
    private fun onError(code: Int, msg: String) {
        errorListener?.onError(code, msg)
    }

    fun setDataSource(url: String) {
        this.url = url;
    }

    fun play() {
        val finalUrl = url ?: throw NullPointerException("url is null! please call setDataSource()")
        nPlay(finalUrl)
    }

    fun release(){
        nRelease()
    }

    private external fun nPlay(url: String)
    private external fun nRelease()
}