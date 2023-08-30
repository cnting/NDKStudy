#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include <opencv2/opencv.hpp>
#include "bitmap_utils.h"
using namespace cv;
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_opencv_opencv_Utils_nBitmap2Mat(JNIEnv *env, jobject thiz, jobject bitmap,
                                                jlong m_native_ptr) {
    Mat *mat = reinterpret_cast<Mat *>(m_native_ptr);
    bitmap2Mat(env, *mat, bitmap);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_opencv_opencv_Utils_nMat2Bitmap(JNIEnv *env, jobject thiz, jlong m_native_ptr,
                                                jobject bitmap) {
    Mat *mat = reinterpret_cast<Mat *>(m_native_ptr);
    mat2Bitmap(env, *mat, bitmap);
}