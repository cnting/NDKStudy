#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include "opencv2/opencv.hpp"
#include "android/log.h"

#define TAG "JNI_TAG"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

using namespace cv;

void bitmap2Mat(JNIEnv *env, Mat &mat, jobject bitmap) {
    //Mat的type：CV_8UC4 ==> ARGB_8888，CV_8UC2 ==> ARGB_565
    //1.获取bitmap信息
    AndroidBitmapInfo info;
    AndroidBitmap_getInfo(env, bitmap, &info);
    void *pixels;

    //锁定bitmap画布
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    //指定mat的宽高和type，mat中是 BGRA
    mat.create(info.height, info.width, CV_8UC4);

    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        //对应的type是CV_8UC4
        Mat temp(info.height, info.width, CV_8UC4, pixels);
        //把数据temp复制到mat
        temp.copyTo(mat);
    } else if (info.format == ANDROID_BITMAP_FORMAT_RGB_565) {
        //对应的type是CV_8UC2
        Mat temp(info.height, info.width, CV_8UC2, pixels);
        //bitmap是RGB_565，mat是BGRA的，所以要用下面方式转
        cvtColor(temp, mat, COLOR_BGR5652BGRA);
    }
    //其他format要自己转

    //解锁Bitmap画布
    AndroidBitmap_unlockPixels(env, bitmap);
}

void mat2Bitmap(JNIEnv *env, const Mat &mat, jobject bitmap) {
    //1.获取bitmap信息
    AndroidBitmapInfo info;
    AndroidBitmap_getInfo(env, bitmap, &info);
    void *pixels;

    //锁定bitmap画布
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        //对应的type是CV_8UC4
        Mat temp(info.height, info.width, CV_8UC4, pixels);
        if (mat.type() == CV_8UC4) {
            mat.copyTo(temp);
        } else if (mat.type() == CV_8UC2) {
            cvtColor(mat, temp, COLOR_BGR5652BGRA);
        } else if (mat.type() == CV_8UC1) { //灰度mat
            cvtColor(mat, temp, COLOR_GRAY2BGRA);
        }
    } else if (info.format == ANDROID_BITMAP_FORMAT_RGB_565) {
        //对应的type是CV_8UC2
        Mat temp(info.height, info.width, CV_8UC2, pixels);
        if (mat.type() == CV_8UC4) {
            cvtColor(mat, temp, COLOR_RGBA2BGR565);
        } else if (mat.type() == CV_8UC2) {
            mat.copyTo(temp);
        } else if (mat.type() == CV_8UC1) { //灰度mat
            cvtColor(mat, temp, COLOR_GRAY2BGR565);
        }
    }
    //其他format要自己转

    //解锁Bitmap画布
    AndroidBitmap_unlockPixels(env, bitmap);
}

/**
 * 加载人脸识别的分类器文件
 */
CascadeClassifier cascadeClassifier;
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_opencv_FaceDetection_loadCascade(JNIEnv *env, jobject thiz, jstring file_path) {
    const char *filePath = env->GetStringUTFChars(file_path, NULL);
    cascadeClassifier.load(filePath);
    LOGE("加载分类器文件成功");
    env->ReleaseStringUTFChars(file_path, filePath);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_cnting_opencv_FaceDetection_faceDetectionSaveInfo(JNIEnv *env, jobject thiz,
                                                           jobject bitmap) {
    //检测人脸，opencv只会处理Mat，不会处理Bitmap，所以要先将Bitmap转Mat
    //1.Bitmap 转 Mat，Mat是个矩阵
    Mat mat;
    bitmap2Mat(env, mat, bitmap);

    //处理灰度，提高效率，一般所有操作都会进行灰度处理
    Mat gray_mat;
    cvtColor(mat, gray_mat, COLOR_BGRA2GRAY);

    //再次处理 直方均衡补偿
    Mat equalize_mat;
    equalizeHist(gray_mat, equalize_mat);

    //识别人脸，要加载人脸分类器文件
    std::vector<Rect> faces;
    cascadeClassifier.detectMultiScale(equalize_mat, faces, 1.1, 5);
    LOGE("人脸个数:%d", faces.size());
    if (faces.size() == 1) {
        //拿到人脸区域
        Rect faceRect = faces[0];

        //在人脸部位画个图
        rectangle(mat, faceRect, Scalar(255, 155, 144), 8);

        mat2Bitmap(env, mat, bitmap);

        //保存人脸信息Mat，保存成 图片 jpg
        Mat fact_info_mat(equalize_mat, faceRect);

        //todo 将fact_info_mat保存成图片
    }

    return 0;
}


/**
 * https://www.bilibili.com/video/BV1Ay4y117HV?p=213&spm_id_from=pageDriver&vd_source=cfa545ca14ba2d7782dd4c30ae22638e
 */
extern "C"
JNIEXPORT int JNICALL
Java_com_cnting_opencv_BitmapUtil_gray3(JNIEnv *env, jobject thiz, jobject src) {
    AndroidBitmapInfo bitmapInfo;
    int info_res = AndroidBitmap_getInfo(env, src, &bitmapInfo);
    if (info_res != 0) return -1;
    void *pixels;
    AndroidBitmap_lockPixels(env, src, &pixels);

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        for (int i = 0; i < bitmapInfo.width * bitmapInfo.height; ++i) {
            uint32_t *pixel_p = reinterpret_cast<uint32_t *>(pixels) + i;
            uint32_t pixel = *pixel_p;
            int a = (pixel >> 24) & 0xff;
            int r = (pixel >> 16) & 0xff;
            int g = (pixel >> 8) & 0xff;
            int b = pixel & 0xff;
            int gray = (int) (0.213f * r + 0.715f * g + 0.072f * b);
            *pixel_p = (a << 24) | (gray << 16) | (gray << 8) | gray;

        }
    } else { //RGB_565，R占5位，G占6位，B占5位
        for (int i = 0; i < bitmapInfo.width * bitmapInfo.height; ++i) {
            uint16_t *pixel_p = reinterpret_cast<uint16_t *>(pixels) + i;
            uint16_t pixel = *pixel_p;
            int r = ((pixel >> 11) & 0x1f) << 3;  //5位
            int g = ((pixel >> 5) & 0x3f) << 2;   //6位
            int b = (pixel & 0x1f) << 3;          //5位

            //上面((pixel >> 11) & 0x1f)是按5位来算的
            //但这个方法是按8位来算的，所以上面方法需要<<3补足8位
            int gray = (int) (0.213f * r + 0.715f * g + 0.072f * b);

            *pixel_p = ((gray >> 3) << 11) | ((gray >> 2) << 5) | (gray >> 3);
        }
    }

    AndroidBitmap_unlockPixels(env, src);
    return 1;
}