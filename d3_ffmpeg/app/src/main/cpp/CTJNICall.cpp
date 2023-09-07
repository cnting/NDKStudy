//
// Created by cnting on 2023/9/7.
//

#include "CTJNICall.h"
#include "constants.h"

CTJNICall::CTJNICall(JavaVM *javaVm, JNIEnv *jniEnv, jobject jPlayerObj) {
    this->javaVM = javaVm;
    this->jniEnv = jniEnv;
    //全局引用可以跨线程
    this->jPlayerObj = jniEnv->NewGlobalRef(jPlayerObj);

    jclass jPlayerClass = jniEnv->FindClass("com/cnting/ffmpeg/media/CTPlayer");
    jPlayerErrorMid = jniEnv->GetMethodID(jPlayerClass, "onError", "(ILjava/lang/String;)V");
    jPlayerPreparedMid = jniEnv->GetMethodID(jPlayerClass, "onPrepared", "()V");
}

CTJNICall::~CTJNICall() {
    jniEnv->DeleteGlobalRef(jPlayerObj);
}


void CTJNICall::callPlayerError(ThreadMode threadMode, int code, char *msg) {
    if (threadMode == THREAD_MAIN) {
        jstring str = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jPlayerObj, jPlayerErrorMid, code, str);
        jniEnv->DeleteLocalRef(str);
    } else {
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("get child thread jniEnv error!");
            return;
        }

        jstring str = env->NewStringUTF(msg);
        env->CallVoidMethod(jPlayerObj, jPlayerErrorMid, code, str);
        env->DeleteLocalRef(str);

        javaVM->DetachCurrentThread();
    }
}

void CTJNICall::callPlayPrepared(ThreadMode threadMode) {
    if (threadMode == THREAD_MAIN) {
        jniEnv->CallVoidMethod(jPlayerObj, jPlayerPreparedMid);
    } else {
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("get child thread jniEnv error!");
            return;
        }

        env->CallVoidMethod(jPlayerObj, jPlayerPreparedMid);

        javaVM->DetachCurrentThread();
    }
}
