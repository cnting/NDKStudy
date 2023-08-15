//
// Created by cnting on 2023/8/15.
//


#ifndef D31_OCR_BITMAPMATUTIL_H
#define D31_OCR_BITMAPMATUTIL_H

#include <jni.h>
#include "opencv2/opencv.hpp"

using namespace cv;

class BitmapMatUtil {
public:
//返回值一般返回是否成功
    static int bitmap2Mat(JNIEnv *env, jobject bitmap, Mat &mat);

    static int mat2Bitmap(JNIEnv *env, jobject bitmap, Mat &mat);
};


#endif //D31_OCR_BITMAPMATUTIL_H
