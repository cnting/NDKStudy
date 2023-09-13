package com.cnting.livepush.livepush

/**
 * Created by cnting on 2023/9/11
 *
 */
interface ConnectListener {
    fun connectError(code: Int, msg: String)
    fun connectSuccess()
}