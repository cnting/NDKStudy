#include <jni.h>
#include <string>
#include "android/log.h"

#define TAG "JNI_TAG"
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
using namespace std;
/**
 * 修改属性
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_jni_MainActivity_changeField(JNIEnv *env, jobject thiz) {
    //获取name属性
    jclass j_clazz = env->GetObjectClass(thiz);
    jfieldID j_fid = env->GetFieldID(j_clazz, "name", "Ljava/lang/String;");
    jstring name = static_cast<jstring>(env->GetObjectField(thiz, j_fid));

    //转c的字符串
    char *c_str = const_cast<char *>(env->GetStringUTFChars(name, NULL));
    LOGE("name is %s", c_str);
    //记得回收！！！
    env->ReleaseStringUTFChars(name, c_str);

    //修改属性值
    jstring newName = env->NewStringUTF("李四");
    env->SetObjectField(thiz, j_fid, newName);
}
/**
 * 获取静态变量并修改
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_jni_MainActivity_changeStaticField(JNIEnv *env, jobject thiz) {
    //获取name属性
    jclass j_clazz = env->GetObjectClass(thiz);
    jfieldID j_fid = env->GetStaticFieldID(j_clazz, "age", "I");
    jint age = env->GetStaticIntField(j_clazz, j_fid);

    LOGE("age is %d", age);

    //修改属性值
    env->SetStaticIntField(j_clazz, j_fid, 88);
}
/**
 * 调用Java方法
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_jni_MainActivity_callJavaMethod(JNIEnv *env, jobject thiz) {
    jclass j_clazz = env->GetObjectClass(thiz);
    jmethodID j_mid = env->GetMethodID(j_clazz, "add", "(II)I");
    jint result = env->CallIntMethod(thiz, j_mid, 10, 20);
    LOGE("调用add(10,20)：%d", result);
}
/**
 * 调用Java静态方法
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_jni_MainActivity_callJavaStaticMethod(JNIEnv *env, jobject thiz) {
    jclass j_clazz = env->GetObjectClass(thiz);
    jmethodID j_mid = env->GetStaticMethodID(j_clazz, "getUUID", "()Ljava/lang/String;");
    jstring result = static_cast<jstring>(env->CallStaticObjectMethod(j_clazz, j_mid));
    const char *c_uuid = env->GetStringUTFChars(result, NULL);
    LOGE("getUUID()：%s", c_uuid);

    //记得回收！！！
    env->ReleaseStringUTFChars(result, c_uuid);
}
/**
 * 构建java对象
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_jni_MainActivity_createNativePoint(JNIEnv *env, jobject thiz) {
    jclass j_clazz = env->FindClass("com/cnting/jni/Point");
    jmethodID j_mid = env->GetMethodID(j_clazz, "<init>", "(II)V");
    jobject point = env->NewObject(j_clazz, j_mid, 10, 20);

    //修改y值
    j_mid = env->GetMethodID(j_clazz, "setY", "(I)V");
    env->CallVoidMethod(point, j_mid, 100);

    return point;
}

/**
 * 数组操作
 */

int compare(jint *a, jint *b) {
    //a和b是指针，要解引用
    return *a - *b;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_jni_MainActivity_sortArray(JNIEnv *env, jobject thiz, jintArray arr) {
    jint *intArray = env->GetIntArrayElements(arr, NULL);
    int len = env->GetArrayLength(arr);

    //static void qsort(void *__base, size_t __nmemb, size_t __size, int (*)(const void *,const void *) __comparator)
    //第一个参数：void * 数组首地址
    //第二个参数：数组长度大小
    //第三个参数：数组元素数据类型的大小
    //第四个参数：函数指针

    qsort(intArray, len, sizeof(int),
          reinterpret_cast<int (*)(const void *, const void *)>(compare));

    //记得同步数组数据!!
    //0:既要同步数据给jarray，又要释放 intArray
    //JNI_COMMIT: 会同步数据给jarray，不会释放 intArray
    //JNI_ABORT:不同步数据给jarray，但会释放 intArray
    env->ReleaseIntArrayElements(arr, intArray, JNI_COMMIT);
}
/**
 * 局部引用
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_jni_MainActivity_localRef(JNIEnv *env, jobject thiz) {
    jclass j_clazz = env->FindClass("java/lang/String");
    jmethodID j_mid = env->GetMethodID(j_clazz, "<init>", "()V");
    jobject j_str = env->NewObject(j_clazz, j_mid);

    //要记得回收
    env->DeleteLocalRef(j_str);
}
/**
 * 全局引用
 */
jstring globalStr;
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_jni_MainActivity_saveGlobalRef(JNIEnv *env, jobject thiz, jstring str) {
    //保存全局变量
    globalStr = static_cast<jstring>(env->NewGlobalRef(str));
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_cnting_jni_MainActivity_getGlobalRef(JNIEnv *env, jobject thiz) {
    return globalStr;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_jni_MainActivity_deleteGlobalRef(JNIEnv *env, jobject thiz) {
    env->DeleteGlobalRef(globalStr);
}
/**
 * 静态缓存策略
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_jni_MainActivity_staticLocalCache(JNIEnv *env, jobject thiz, jstring str) {
    jclass j_clazz = env->GetObjectClass(thiz);
    //声明为static变量进行局部缓存
    static jfieldID j_fid = NULL;
    if (j_fid == NULL) {
        j_fid = env->GetStaticFieldID(j_clazz, "staticName", "Ljava/lang/String;");
    } else {
        LOGE("fieldId 不为空");
    }

    env->SetStaticObjectField(j_clazz, j_fid, str);
}
/**
 * 全局缓存，在初始化时进行缓存
 */
static jfieldID static_f_id1 = NULL;
static jfieldID static_f_id2 = NULL;
static jfieldID static_f_id3 = NULL;
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_jni_MainActivity_initStaticCache(JNIEnv *env, jobject thiz) {
    //这个方法在初始化时被调用

    //在这里给 static_f_id1、static_f_id2、static_f_id3 赋值
}

/**
 * 异常处理
 * 1.在c++层如果是自己写的代码或调用别人的代码，记得try住，如果不try，在Java层try是没用的
 * 2.如果异常需要抛给Java层，一定要按照Java层抛异常的方式
 * 3.如果是自己写的NDK的代码，最好抛自己写的异常
 * 4.如果是c/c++写代码，最好抛系统异常 或 继承系统异常
 */

/**
 * 自定义异常，继承out_of_range，这个是系统异常
 */
class Exception : public out_of_range {
public:
    Exception(string msg) : out_of_range(msg) {
        this->msg = msg;
    }

    const char *what() {
        return this->msg.c_str();
    }

    string msg;
};

//如果代码中会抛异常，最好要在方法名上声明可能抛的异常
void c_method() throw(Exception, int) {
    throw Exception("异常了");
}

void c_method1() {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_jni_MainActivity_exception(JNIEnv *env, jobject thiz) {
    //native出错了，是没法在java层是没法被try catch的
    //比如这里拿一个不存在的变量
//    jclass j_clazz = env->GetObjectClass(thiz);
//    jfieldID j_fid = env->GetFieldID(j_clazz, "name3", "Ljava/lang/String;");

    //方式1：补救措施，拿不到name3 就 拿name
    //判断是否出现异常
//    jthrowable throwable = env->ExceptionOccurred();
//    if(throwable){
//        //要记得先把异常清除!!!
//        env->ExceptionClear();
//        LOGE("有异常");
//        //然后重新获取 name 属性
//        j_fid = env->GetFieldID(j_clazz, "name", "Ljava/lang/String;");
//    }

    //方式2：想给Java层抛异常
//    jthrowable throwable = env->ExceptionOccurred();
//    if (throwable) {
//        //要记得先把异常清除!!!
//        env->ExceptionClear();
//        jclass no_such_clz = env->FindClass("java/lang/NoSuchFieldException");
//        env->ThrowNew(no_such_clz, "NoSuchFieldException name3");
//        //记得return !!!
//        return;
//    }
//    jstring name = static_cast<jstring>(env->GetObjectField(thiz, j_fid));


    //方式3：try catch住native层异常，并给Java层抛异常
    try {
        c_method();
    } catch (const Exception &exception) {
        //按Java层抛异常方式
        jclass no_such_clz = env->FindClass("java/lang/NoSuchFieldException");
        env->ThrowNew(no_such_clz, exception.msg.c_str());
    }
}
/**
 * 仿System.arraycopy()
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_jni_MainActivity_arraycopy(JNIEnv *env, jobject thiz, jobject src, jint src_pos,
                                           jobject dest, jint dest_pos, jint length) {

    //做一些判断是否是数组
    jobjectArray src_array = static_cast<jobjectArray>(src);
    jobjectArray dest_array = static_cast<jobjectArray>(dest);

    //转换失败抛异常
    if (src_array == NULL || dest_array == NULL) {
        LOGE("转换失败");
        return;
    }

    for (int i = src_pos; i < src_pos + length; i++) {
        jobject obj = env->GetObjectArrayElement(src_array, i);
        env->SetObjectArrayElement(dest_array, i, obj);
    }
}