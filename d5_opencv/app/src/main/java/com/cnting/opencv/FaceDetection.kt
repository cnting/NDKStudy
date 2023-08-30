package com.cnting.opencv

import android.graphics.Bitmap

/**
 * Created by cnting on 2023/8/9
 *
 */
class FaceDetection {
    external fun faceDetectionBitmap(bitmap: Bitmap): Int
    external fun faceDetection(matPtr: Long): Int

    /**
     * 加载人脸识别的分类器文件
     */
    external fun loadCascade(filePath: String)


}