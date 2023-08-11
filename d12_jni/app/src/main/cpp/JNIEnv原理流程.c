//https://www.bilibili.com/video/BV1Ay4y117HV/?p=47&spm_id_from=pageDriver&vd_source=cfa545ca14ba2d7782dd4c30ae22638e

//JNIEnv是JNINativeInterface这个结构体的指针别名
typedef const struct JNINativeInterface *JNIEnv;

/**
 * 模拟一个结构体
 */
struct JNINativeInterface {
    //这是一个函数指针，函数指针的定义：
    //方法返回值类型(*方法的名称)(方法的参数)
    //函数指针名就是 NewStringUTF
    char *(*NewStringUTF)(JNIEnv *, char *);
};

char *NewStringUTF(JNIEnv *env, char *c_str) {
    //c_str -> jstring
    return c_str;
}

char *Java_com_cnting_jni_MainActivity_stringFromJNI1(
        JNIEnv *env) {
    char *str = "Hello from C++";
    return (*env)->NewStringUTF(env, str);
}

int main() {
    //构建JNIEnv* 对象
    struct JNINativeInterface nativeInterface;
    nativeInterface.NewStringUTF = NewStringUTF;

    JNIEnv env = &nativeInterface;  //一级指针
    JNIEnv *jniEnv = &env;         //二级指针

    char * jstr = Java_com_cnting_jni_MainActivity_stringFromJNI1(jniEnv);

    //jstr通过JNIEnv传给java桥梁
}