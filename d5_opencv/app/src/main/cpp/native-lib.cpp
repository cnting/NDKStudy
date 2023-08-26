#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include "opencv2/opencv.hpp"
#include "android/log.h"
#include "bitmap_utils.h"

#define TAG "JNI_TAG"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

using namespace cv;

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
/**
 * 逆世界
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_inverseWorld(JNIEnv *env, jobject thiz, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    Mat res(src.size(), src.type());

    int src_w = src.cols;
    int src_h = src.rows;
    int mid_h = src_h >> 1;
    int a_h = mid_h >> 1;
    //处理下半部分
    for (int row = 0; row < mid_h; row++) {
        for (int col = 0; col < src_w; col++) {
            //现在type是BGRA，所以用Vec4b接收；如果type是BGR，就要用Vec3b接收
            res.at<Vec4b>(row + mid_h, col) = src.at<Vec4b>(row + a_h, col);
        }
    }
    for (int row = 0; row < mid_h; row++) {
        for (int col = 0; col < src_w; col++) {
            res.at<Vec4b>(row, col) = src.at<Vec4b>(src_h - a_h - row, col);
        }
    }
    mat2Bitmap(env, res, bitmap);
    return bitmap;
}
/**
 * 镜像
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_mirror(JNIEnv *env, jobject thiz, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    Mat res(src.size(), src.type());

    int src_w = src.cols;
    int src_h = src.rows;
    int mid_w = src_w >> 1;
    int a_w = mid_w >> 1;
    for (int row = 0; row < src_h; row++) {
        for (int col = 0; col < mid_w; col++) {
            res.at<Vec4b>(row, col + mid_w) = src.at<Vec4b>(row, col + a_w);
        }
    }
    for (int row = 0; row < src_h; row++) {
        for (int col = 0; col < mid_w; col++) {
            res.at<Vec4b>(row, col) = src.at<Vec4b>(row, src_w - a_w - col);
        }
    }
    mat2Bitmap(env, res, bitmap);
    return bitmap;
}
/**
 * 浮雕效果
 * 有立体感，突出轮廓信息，使用下面算子来做卷积
 * [1,0
 * 0,-1]
 * 这个算子相当于左上角的像素减右下角的像素
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_anaglyph(JNIEnv *env, jobject thiz, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    cvtColor(src, src, COLOR_BGRA2GRAY);

    Mat res(src.size(), src.type());

    int src_w = src.cols;
    int src_h = src.rows;
    for (int row = 0; row < src_h - 1; row++) {
        for (int col = 0; col < src_w - 1; col++) {
            //左上角像素
            Vec4b current = src.at<Vec4b>(row, col);
            //右下角像素
            Vec4b next = src.at<Vec4b>(row + 1, col + 1);
            //浮雕效果要加128
            res.at<Vec4b>(row, col)[0] = saturate_cast<uchar>(current[0] - next[0] + 128);
            res.at<Vec4b>(row, col)[1] = saturate_cast<uchar>(current[1] - next[1] + 128);
            res.at<Vec4b>(row, col)[2] = saturate_cast<uchar>(current[2] - next[1] + 128);
            res.at<Vec4b>(row, col)[3] = current[3];
        }
    }
    mat2Bitmap(env, res, bitmap);
    return bitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_mosaic(JNIEnv *env, jobject thiz, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    int src_w = src.cols;
    int src_h = src.rows;

    //省略人脸识别，只对人脸位置打马赛克，假设下面区域是人脸
    int row_s = src_h >> 2;
    int row_e = src_h * 3 / 4;
    int col_s = src_w >> 2;
    int col_e = src_w * 3 / 4;
    //马赛克大小
    int size = 30;

    for (int row = row_s; row < row_e; row += size) {
        for (int col = col_s; col < col_e; col += size) {
            int pixel = src.at<int>(row, col);
            for (int m_rows = 1; m_rows < size; m_rows++) {
                for (int m_cols = 1; m_cols < size; m_cols++) {
                    src.at<int>(row + m_rows, col + m_cols) = pixel;
                }
            }
        }
    }
    mat2Bitmap(env, src, bitmap);
    return bitmap;
}