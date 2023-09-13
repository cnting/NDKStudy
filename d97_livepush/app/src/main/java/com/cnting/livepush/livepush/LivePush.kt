package com.cnting.livepush.livepush

import android.os.Handler
import android.os.Looper


/**
 * Created by cnting on 2023/9/11
 *
 */
class LivePush(private val liveUrl: String) {
    companion object {
        init {
            System.loadLibrary("livepush")
        }
    }

    private val handler = Handler(Looper.getMainLooper())
    var connectListener: ConnectListener? = null

    fun initConnect() {
        nInitConnect(liveUrl)
    }

    fun stop() {
        handler.post { nStop() }
    }

    //call from jni
    private fun onConnectError(code: Int, msg: String) {
        stop()
        connectListener?.connectError(code, msg)
    }

    //call from jni
    private fun onConnectSuccess() {
        connectListener?.connectSuccess()
    }

    private external fun nInitConnect(liveUrl: String)
    private external fun nStop()
}