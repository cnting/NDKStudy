#include "bitmap_utils.h"

void bitmap2Mat(JNIEnv *env, Mat &mat, jobject &bitmap) {
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

void mat2Bitmap(JNIEnv *env, const Mat &mat, jobject &bitmap) {
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

jobject createBitmap(JNIEnv *env, int w, int h, int type) {
    char *config_name;
    if (type == CV_8UC4) {
        config_name = "ARGB_8888";
    } else {
        config_name = "RGB_565";
    }
    jstring config_name_str = env->NewStringUTF(config_name);
    jclass java_bitmap_config_class = env->FindClass("android/graphics/Bitmap$Config");
    jmethodID mid = env->GetStaticMethodID(java_bitmap_config_class, "valueOf",
                                           "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
    jobject config = env->CallStaticObjectMethod(java_bitmap_config_class, mid, config_name_str);

    jclass java_bitmap_class = env->FindClass("android/graphics/Bitmap");//类名
    mid = env->GetStaticMethodID(java_bitmap_class, "createBitmap",//获取方法
                                 "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jobject bitmap = env->CallStaticObjectMethod(java_bitmap_class, mid, w, h,
                                                 config);
    return bitmap;
}


