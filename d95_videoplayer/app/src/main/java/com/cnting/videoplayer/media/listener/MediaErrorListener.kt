package com.cnting.videoplayer.media.listener

/**
 * Created by cnting on 2023/9/7
 *
 */
interface MediaErrorListener {
    fun onError(code: Int, msg: String)
}