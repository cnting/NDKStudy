#include <jni.h>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
}

/**
 * @param argc 命令的个数
 * @param argv 二维数组
 * @return
 */
int ffmpegmain(int argc, char **argv);

extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_ffmpeg_VideoCompress_compressVideo(JNIEnv *env, jobject thiz,
                                                   jobjectArray compress_command,
                                                   jobject callback) {
    //1.获取命令个数
    int argc = env->GetArrayLength(compress_command);

    //2.给 char**argv填充数据
    char **argv = (char **) malloc(sizeof(char *) * argc);
    for (int i = 0; i < argc; i++) {
        jstring j_param = static_cast<jstring>(env->GetObjectArrayElement(compress_command, i));
        argv[i] = (char *) env->GetStringUTFChars(j_param, NULL);
    }

    //3.调用命令去压缩
    ffmpegmain(argc, argv);

    //4.释放内存
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
}