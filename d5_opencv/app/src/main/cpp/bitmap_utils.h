#ifndef REC_UTILS_H
#define REC_UTILS_H


#include <android/bitmap.h>
#include <opencv2/opencv.hpp>

using namespace cv;//Mat

extern "C" {
/**
 * Bitmap 转矩阵
 * @param env JNI环境
 * @param bitmap Bitmap对象
 * @param mat 图片矩阵
 */
void bitmap2Mat(JNIEnv *env, Mat &mat, jobject &bitmap);

/**
 * 矩阵转Bitmap
 * @param env JNI环境
 * @param mat 图片矩阵
 * @param bitmap Bitmap对象
 */
void mat2Bitmap(JNIEnv *env, const Mat &mat, jobject &bitmap);

/**
 *
 * 创建Bitmap
 * @param env JNI环境
 * @param src 矩阵
 * @param config Bitmap配置
 * @return Bitmap对象
 */
jobject createBitmap(JNIEnv *env, int w,int h, int type);
}

#endif //REC_UTILS_H
