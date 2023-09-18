#include <jni.h>
#include <string>
#include "CTLivePush.h"
#include "constants.h"


CTLivePush *pLivePush = NULL;
CTJNICall *pJniCall = NULL;
JavaVM *pJavaVm = NULL;

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_livepush_livepush_LivePush_nInitConnect(JNIEnv *env, jobject thiz,
                                                        jstring live_url) {
    const char *url = env->GetStringUTFChars(live_url, 0);

    env->GetJavaVM(&pJavaVm);
    pJniCall = new CTJNICall(pJavaVm, env, thiz);
    pLivePush = new CTLivePush(url, pJniCall);
    pLivePush->initConnect();

    env->ReleaseStringUTFChars(live_url, url);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_livepush_livepush_LivePush_nStop(JNIEnv *env, jobject thiz) {
    LOGE("nStop()");
    if (pLivePush != NULL) {
        pLivePush->stop();
        delete pLivePush;
        pLivePush = NULL;
    }
    if (pJniCall != NULL) {
        delete pJniCall;
        pJniCall = NULL;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_livepush_livepush_LivePush_pushSpsPPs(JNIEnv *env, jobject thiz,
                                                      jbyteArray video_sps, jint sps_length,
                                                      jbyteArray video_pps, jint pps_length) {
    jbyte *spsData = env->GetByteArrayElements(video_sps, NULL);
    jbyte *ppsData = env->GetByteArrayElements(video_pps, NULL);
    if (pLivePush != NULL) {
        pLivePush->pushSpsPps(spsData, sps_length, ppsData, pps_length);
    }
    env->ReleaseByteArrayElements(video_sps, spsData, 0);
    env->ReleaseByteArrayElements(video_pps, ppsData, 0);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_livepush_livepush_LivePush_pushVideo(JNIEnv *env, jobject thiz,
                                                     jbyteArray video_data, jint length,
                                                     jboolean is_key_frame) {
    jbyte *videoData = env->GetByteArrayElements(video_data, NULL);
    if(pLivePush!=NULL){
        pLivePush->pushVideo(videoData,length,is_key_frame);
    }

    env->ReleaseByteArrayElements(video_data, videoData, 0);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_livepush_livepush_LivePush_pushAudio(JNIEnv *env, jobject thiz,
                                                     jbyteArray audio_data, jint data_len) {
    jbyte *videoData = env->GetByteArrayElements(audio_data, NULL);
    if(pLivePush!=NULL){
        pLivePush->pushAudio(videoData,data_len);
    }

    env->ReleaseByteArrayElements(audio_data, videoData, 0);
}