//
// Created by cnting on 2023/9/7.
//

#include "CTJNICall.h"
#include "constants.h"

CTJNICall::CTJNICall(JavaVM *javaVm, JNIEnv *jniEnv, jobject jLiveObj) {
    this->javaVM = javaVm;
    this->jniEnv = jniEnv;
    //全局引用可以跨线程
    this->jLiveObj = jniEnv->NewGlobalRef(jLiveObj);

    jclass jPlayerClass = jniEnv->GetObjectClass(jLiveObj);
    jConnectErrorMid = jniEnv->GetMethodID(jPlayerClass, "onConnectError", "(ILjava/lang/String;)V");
    jConnectSuccessMid = jniEnv->GetMethodID(jPlayerClass, "onConnectSuccess", "()V");
}

CTJNICall::~CTJNICall() {
    jniEnv->DeleteGlobalRef(jLiveObj);
}


void CTJNICall::callConnectError(ThreadMode threadMode, int code, char *msg) {
    if (threadMode == THREAD_MAIN) {
        jstring str = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jLiveObj, jConnectErrorMid, code, str);
        jniEnv->DeleteLocalRef(str);
    } else {
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("get child thread jniEnv error!");
            return;
        }

        jstring str = env->NewStringUTF(msg);
        env->CallVoidMethod(jLiveObj, jConnectErrorMid, code, str);
        env->DeleteLocalRef(str);

        javaVM->DetachCurrentThread();
    }
}

void CTJNICall::callConnectSuccess(ThreadMode threadMode) {
    if (threadMode == THREAD_MAIN) {
        jniEnv->CallVoidMethod(jLiveObj, jConnectSuccessMid);
    } else {
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("get child thread jniEnv error!");
            return;
        }

        env->CallVoidMethod(jLiveObj, jConnectSuccessMid);

        javaVM->DetachCurrentThread();
    }
}
