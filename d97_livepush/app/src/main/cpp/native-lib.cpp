#include <jni.h>
#include <string>
#include "CTLivePush.h"


CTLivePush *pLivePush = NULL;
CTJNICall *pJniCall = NULL;
JavaVM *pJavaVm = NULL;

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_livepush_LivePush_nInitConnect(JNIEnv *env, jobject thiz, jstring live_url) {
    const char *url = env->GetStringUTFChars(live_url, 0);

    env->GetJavaVM(&pJavaVm);
    pJniCall = new CTJNICall(pJavaVm, env, thiz);
    pLivePush = new CTLivePush(url, pJniCall);
    pLivePush->initConnect();

    env->ReleaseStringUTFChars(live_url, url);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_livepush_LivePush_nStop(JNIEnv *env, jobject thiz) {
    if (pLivePush != NULL) {
        delete pLivePush;
        pLivePush = NULL;
    }
    if (pJniCall != NULL) {
        delete pJniCall;
        pJniCall = NULL;
    }
}