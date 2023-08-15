//
// Created by cnting on 2023/8/15.
//

#include "BitmapMatUtil.h"
#include "android/bitmap.h"

using namespace cv;

int BitmapMatUtil::bitmap2Mat(JNIEnv *env, jobject bitmap, Mat &mat) {
    //1.锁定画布
    void *pixels;
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    //获取bitmap信息
    AndroidBitmapInfo bitmapInfo;
    AndroidBitmap_getInfo(env, bitmap, &bitmapInfo);

    //CV_8UC4 -> argb
    //CV_8UC2 -> rgb
    //CV_8UC1->gray

    // 返回三通道
    Mat createMat(bitmapInfo.height, bitmapInfo.width, CV_8UC4);

    if (bitmapInfo.format == AndroidBitmapFormat::ANDROID_BITMAP_FORMAT_RGBA_8888) {//四颜色通道
        Mat temp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, pixels);
        temp.copyTo(createMat);
    } else if (bitmapInfo.format == AndroidBitmapFormat::ANDROID_BITMAP_FORMAT_RGB_565) { //三颜色通道
        Mat temp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, pixels);
        cvtColor(temp, createMat, COLOR_BGR5652BGRA);

    }
    createMat.copyTo(mat);

    //2.解锁画布
    AndroidBitmap_unlockPixels(env, bitmap);

    return 0;
}

int BitmapMatUtil::mat2Bitmap(JNIEnv *env, jobject bitmap, Mat &mat) {
 return 0;
}
