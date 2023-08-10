package com.cnting.md5

import android.content.Context

/**
 * Created by cnting on 2023/8/9
 *
 */
object SignatureUtils {

    /**
     * 参数使用md5加密
     * 但是这个方法不能被混淆，所以可以被破解，所以要签名校验
     */
    external fun signatureParams(params: String): String

    /**
     * 签名校验：只允许自己App使用so
     * 但这样其实还不够，还要避免xposed调试，Native线程轮询 tracep_id
     */
    external fun signatureVerify(context: Context)

    init {
        System.loadLibrary("md5")
    }
}

