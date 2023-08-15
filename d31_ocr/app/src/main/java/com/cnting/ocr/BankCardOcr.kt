package com.cnting.ocr

import android.graphics.Bitmap

/**
 * Created by cnting on 2023/8/15
 *
 */
class BankCardOcr {

    external fun cardOcr(bitmap:Bitmap): String

    companion object {
        init {
            System.loadLibrary("ocr")
        }
    }
}