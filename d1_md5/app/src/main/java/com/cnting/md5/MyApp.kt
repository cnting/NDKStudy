package com.cnting.md5

import android.app.Application

/**
 * Created by cnting on 2023/8/9
 *
 */
class MyApp : Application() {

    override fun onCreate() {
        super.onCreate()
        SignatureUtils.signatureVerify(this);
    }
}