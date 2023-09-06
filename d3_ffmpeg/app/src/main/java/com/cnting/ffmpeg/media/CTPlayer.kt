package com.cnting.ffmpeg.media

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

    fun setDataSource(url: String) {
        this.url = url;
    }

    fun play() {
        val finalUrl = url ?: throw NullPointerException("url is null! please call setDataSource()")
        nPlay(finalUrl)
    }

    private external fun nPlay(url: String)
}