package com.cnting.ffmpeg

/**
 * Created by cnting on 2023/8/9
 *
 */
class VideoCompress() {

    external fun compressVideo(compressCommand: Array<String>, callback: CompressCallback)

    companion object {
        init {
            System.loadLibrary("ffmpeg")
        }
    }

    interface CompressCallback {
        fun onCompress(current: Int, total: Int)
    }
}
