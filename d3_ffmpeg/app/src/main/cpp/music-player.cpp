#include <jni.h>
#include <string>
#include "constants.h"
#include "CTJNICall.h"
#include "CTFFmpeg.h"
//在c++中采用c的这种编译方式
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
};


CTJNICall *pJniCall = NULL;
CTFFmpeg *pFFmpeg = NULL;
JavaVM *pJavaVm = NULL;

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_ffmpeg_media_CTPlayer_nPlay(JNIEnv *env, jobject thiz, jstring url_) {
    env->GetJavaVM(&pJavaVm);
    pJniCall = new CTJNICall(pJavaVm, env, thiz);
    const char *url = env->GetStringUTFChars(url_, 0);

    pFFmpeg = new CTFFmpeg(pJniCall, url);
    pFFmpeg->play();

    env->ReleaseStringUTFChars(url_, url);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_ffmpeg_media_CTPlayer_nRelease(JNIEnv *env, jobject thiz) {
    if (pJniCall != NULL) {
        delete pJniCall;
        pJniCall = NULL;
    }
    if (pFFmpeg != NULL) {
        delete pFFmpeg;
        pFFmpeg = NULL;
    }
}