#include <jni.h>
#include <string>
#include "BitmapMatUtil.h"
#include "android/log.h"
#include "CardOcr.h"

#define TAG "JNI_TAG"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

extern "C"
JNIEXPORT jstring JNICALL
Java_com_cnting_ocr_BankCardOcr_cardOcr(JNIEnv *env, jobject thiz, jobject bitmap) {
    //1.bitmap -> mat
    Mat mat;
    BitmapMatUtil::bitmap2Mat(env, bitmap, mat);

    //2.轮廓增强
    Rect card_area;
    co1::find_card_area(mat, card_area);

    //对我们过滤到的银行卡区域进行裁剪
    Mat card_mat(mat, card_area);
    imwrite("/storage/emulated/0/Android/data/com.cnting.ocr/clip.jpg", card_mat);

    //3.截取卡号
    Rect card_number_rect;
    co1::find_card_number_area(card_mat, card_number_rect);
    Mat card_number_mat(card_mat, card_number_rect);
    imwrite("/storage/emulated/0/Android/data/com.cnting.ocr/card_number.jpg", card_number_mat);

    vector<Mat> numbers;
    co1::find_card_numbers(card_number_mat,numbers);

    return env->NewStringUTF("345678");
}