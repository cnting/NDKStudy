#include <jni.h>
#include <string>
#include "constants.h"
#include "CTJNICall.h"
#include "CTFFmpeg.h"
#include "android/native_window.h"
#include "android/native_window_jni.h"

//在c++中采用c的这种编译方式
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"

};


CTJNICall *pJniCall = NULL;
CTFFmpeg *pFFmpeg = NULL;
JavaVM *pJavaVm = NULL;

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_videoplayer_media_CTPlayer_nPlay(JNIEnv *env, jobject thiz) {
    if (pFFmpeg != NULL) {
        pFFmpeg->play();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_videoplayer_media_CTPlayer_nPrepare(JNIEnv *env, jobject thiz, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    if (pFFmpeg == NULL) {
        env->GetJavaVM(&pJavaVm);
        pJniCall = new CTJNICall(pJavaVm, env, thiz);
        pFFmpeg = new CTFFmpeg(pJniCall, url);
        pFFmpeg->prepare();
    }
    env->ReleaseStringUTFChars(url_, url);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_videoplayer_media_CTPlayer_nPrepareAsync(JNIEnv *env, jobject thiz, jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);
    if (pFFmpeg == NULL) {
        env->GetJavaVM(&pJavaVm);
        pJniCall = new CTJNICall(pJavaVm, env, thiz);
        pFFmpeg = new CTFFmpeg(pJniCall, url);
        pFFmpeg->preparedAsync();
    }
    env->ReleaseStringUTFChars(url_, url);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_videoplayer_media_CTPlayer_nRelease(JNIEnv *env, jobject thiz) {
    if (pFFmpeg != NULL) {
        delete pFFmpeg;
        pFFmpeg = NULL;
    }
    if (pJniCall != NULL) {
        delete pJniCall;
        pJniCall = NULL;
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_videoplayer_media_CTPlayer_nStop(JNIEnv *env, jobject thiz) {
}


extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_videoplayer_media_CTPlayer_nSetSurface(JNIEnv *env, jobject thiz, jobject surface) {
    if (pFFmpeg != NULL) {
        pFFmpeg->setSurface(surface);
    }
}