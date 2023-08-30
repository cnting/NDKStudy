package com.cnting.opencv

import android.graphics.Bitmap

/**
 * Created by cnting on 2023/8/30
 *
 */
object ObjDetectUtil {
    external fun hogBitmap(bitmap: Bitmap): Bitmap
    external fun lbpBitmap(bitmap: Bitmap): Bitmap
}