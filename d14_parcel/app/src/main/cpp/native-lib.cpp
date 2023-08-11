#include <jni.h>
#include <string>


class Parcel {
    //共享内存的首地址
    char *mData;
    int mDataPos = 0;

public:
    Parcel() {
        mData = static_cast<char *>(malloc(1024));
    }

    void writeInt(jint value) {
        //mData + mDataPos：指针移动位置
        //*(mData + mDataPos) 赋值
        *(mData + mDataPos) = value;
        mDataPos += sizeof(int);
    }

    jint readInt() {
        int result = *(mData + mDataPos);
        mDataPos += sizeof(int);
        return result;
    }

    void setDataPosition(jint position) {
        mDataPos = position;
    }

    ~Parcel() {
        free(mData);
    }


};

/**
 * 构建Parcel对象，返回指针地址
 */
extern "C"
JNIEXPORT jlong JNICALL
Java_com_cnting_parcel_Parcel_nativeCreate(JNIEnv *env, jobject thiz) {
    Parcel *parcel = new Parcel();
    return reinterpret_cast<jlong>(parcel);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_parcel_Parcel_nativeWriteInt(JNIEnv *env, jobject thiz, jlong m_native_ptr,
                                             jint value) {
    Parcel *parcel = reinterpret_cast<Parcel *>(m_native_ptr);
    parcel->writeInt(value);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_cnting_parcel_Parcel_nativeReadInt(JNIEnv *env, jobject thiz, jlong native_ptr) {
    Parcel *parcel = reinterpret_cast<Parcel *>(native_ptr);
    return parcel->readInt();
}
/**
 * 修改位置
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_parcel_Parcel_nativeSetDataPosition(JNIEnv *env, jobject thiz, jlong native_ptr,
                                                    jint pos) {
    Parcel *parcel = reinterpret_cast<Parcel *>(native_ptr);
    parcel->setDataPosition(pos);
}
