#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include <opencv2/opencv.hpp>

using namespace cv;
extern "C"
JNIEXPORT jlong JNICALL
Java_com_cnting_opencv_opencv_Mat_nMatIII(JNIEnv *env, jobject thiz, jint rows, jint cols,
                                          jint value) {
    Mat *mat = new Mat(rows, cols, value);
    return reinterpret_cast<jlong>(mat);
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_cnting_opencv_opencv_Mat_nMat(JNIEnv *env, jobject thiz) {
    Mat *mat = new Mat();
    return reinterpret_cast<jlong>(mat);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_opencv_opencv_Mat_nDelete(JNIEnv *env, jobject thiz, jlong m_native_ptr) {
    Mat *mat = new Mat();
    delete mat;
}