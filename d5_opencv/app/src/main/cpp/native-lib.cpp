#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include "opencv2/opencv.hpp"
#include "android/log.h"
#include "bitmap_utils.h"

#define TAG "JNI_TAG"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

using namespace cv;

/**
 * 加载人脸识别的分类器文件
 */
CascadeClassifier cascadeClassifier;
extern "C"
JNIEXPORT void JNICALL
Java_com_cnting_opencv_FaceDetection_loadCascade(JNIEnv *env, jobject thiz, jstring file_path) {
    const char *filePath = env->GetStringUTFChars(file_path, NULL);
    cascadeClassifier.load(filePath);
    LOGE("加载分类器文件成功");
    env->ReleaseStringUTFChars(file_path, filePath);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_cnting_opencv_FaceDetection_faceDetectionSaveInfo(JNIEnv *env, jobject thiz,
                                                           jobject bitmap) {
    //检测人脸，opencv只会处理Mat，不会处理Bitmap，所以要先将Bitmap转Mat
    //1.Bitmap 转 Mat，Mat是个矩阵
    Mat mat;
    bitmap2Mat(env, mat, bitmap);

    //处理灰度，提高效率，一般所有操作都会进行灰度处理
    Mat gray_mat;
    cvtColor(mat, gray_mat, COLOR_BGRA2GRAY);

    //再次处理 直方均衡补偿
    Mat equalize_mat;
    equalizeHist(gray_mat, equalize_mat);

    //识别人脸，要加载人脸分类器文件
    std::vector<Rect> faces;
    cascadeClassifier.detectMultiScale(equalize_mat, faces, 1.1, 5);
    LOGE("人脸个数:%d", faces.size());
    if (faces.size() == 1) {
        //拿到人脸区域
        Rect faceRect = faces[0];

        //在人脸部位画个图
        rectangle(mat, faceRect, Scalar(255, 155, 144), 8);

        mat2Bitmap(env, mat, bitmap);

        //保存人脸信息Mat，保存成 图片 jpg
        Mat fact_info_mat(equalize_mat, faceRect);

        //todo 将fact_info_mat保存成图片
    }

    return 0;
}


/**
 * https://www.bilibili.com/video/BV1Ay4y117HV?p=213&spm_id_from=pageDriver&vd_source=cfa545ca14ba2d7782dd4c30ae22638e
 */
extern "C"
JNIEXPORT int JNICALL
Java_com_cnting_opencv_BitmapUtil_gray3(JNIEnv *env, jobject thiz, jobject src) {
    AndroidBitmapInfo bitmapInfo;
    int info_res = AndroidBitmap_getInfo(env, src, &bitmapInfo);
    if (info_res != 0) return -1;
    void *pixels;
    AndroidBitmap_lockPixels(env, src, &pixels);

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        for (int i = 0; i < bitmapInfo.width * bitmapInfo.height; ++i) {
            uint32_t *pixel_p = reinterpret_cast<uint32_t *>(pixels) + i;
            uint32_t pixel = *pixel_p;
            int a = (pixel >> 24) & 0xff;
            int r = (pixel >> 16) & 0xff;
            int g = (pixel >> 8) & 0xff;
            int b = pixel & 0xff;
            int gray = (int) (0.213f * r + 0.715f * g + 0.072f * b);
            *pixel_p = (a << 24) | (gray << 16) | (gray << 8) | gray;

        }
    } else { //RGB_565，R占5位，G占6位，B占5位
        for (int i = 0; i < bitmapInfo.width * bitmapInfo.height; ++i) {
            uint16_t *pixel_p = reinterpret_cast<uint16_t *>(pixels) + i;
            uint16_t pixel = *pixel_p;
            int r = ((pixel >> 11) & 0x1f) << 3;  //5位
            int g = ((pixel >> 5) & 0x3f) << 2;   //6位
            int b = (pixel & 0x1f) << 3;          //5位

            //上面((pixel >> 11) & 0x1f)是按5位来算的
            //但这个方法是按8位来算的，所以上面方法需要<<3补足8位
            int gray = (int) (0.213f * r + 0.715f * g + 0.072f * b);

            *pixel_p = ((gray >> 3) << 11) | ((gray >> 2) << 5) | (gray >> 3);
        }
    }

    AndroidBitmap_unlockPixels(env, src);
    return 1;
}
/**
 * 逆世界
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_inverseWorld(JNIEnv *env, jobject thiz, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    Mat res(src.size(), src.type());

    int src_w = src.cols;
    int src_h = src.rows;
    int mid_h = src_h >> 1;
    int a_h = mid_h >> 1;
    //处理下半部分
    for (int row = 0; row < mid_h; row++) {
        for (int col = 0; col < src_w; col++) {
            //现在type是BGRA，所以用Vec4b接收；如果type是BGR，就要用Vec3b接收
            res.at<Vec4b>(row + mid_h, col) = src.at<Vec4b>(row + a_h, col);
        }
    }
    for (int row = 0; row < mid_h; row++) {
        for (int col = 0; col < src_w; col++) {
            res.at<Vec4b>(row, col) = src.at<Vec4b>(src_h - a_h - row, col);
        }
    }
    mat2Bitmap(env, res, bitmap);
    return bitmap;
}
/**
 * 镜像
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_mirror(JNIEnv *env, jobject thiz, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    Mat res(src.size(), src.type());

    int src_w = src.cols;
    int src_h = src.rows;
    int mid_w = src_w >> 1;
    int a_w = mid_w >> 1;
    for (int row = 0; row < src_h; row++) {
        for (int col = 0; col < mid_w; col++) {
            res.at<Vec4b>(row, col + mid_w) = src.at<Vec4b>(row, col + a_w);
        }
    }
    for (int row = 0; row < src_h; row++) {
        for (int col = 0; col < mid_w; col++) {
            res.at<Vec4b>(row, col) = src.at<Vec4b>(row, src_w - a_w - col);
        }
    }
    mat2Bitmap(env, res, bitmap);
    return bitmap;
}
/**
 * 浮雕效果
 * 有立体感，突出轮廓信息，使用下面算子来做卷积
 * [1,0
 * 0,-1]
 * 这个算子相当于左上角的像素减右下角的像素
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_anaglyph(JNIEnv *env, jobject thiz, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    cvtColor(src, src, COLOR_BGRA2GRAY);

    Mat res(src.size(), src.type());

    int src_w = src.cols;
    int src_h = src.rows;
    for (int row = 0; row < src_h - 1; row++) {
        for (int col = 0; col < src_w - 1; col++) {
            //左上角像素
            Vec4b current = src.at<Vec4b>(row, col);
            //右下角像素
            Vec4b next = src.at<Vec4b>(row + 1, col + 1);
            //浮雕效果要加128
            res.at<Vec4b>(row, col)[0] = saturate_cast<uchar>(current[0] - next[0] + 128);
            res.at<Vec4b>(row, col)[1] = saturate_cast<uchar>(current[1] - next[1] + 128);
            res.at<Vec4b>(row, col)[2] = saturate_cast<uchar>(current[2] - next[1] + 128);
            res.at<Vec4b>(row, col)[3] = current[3];
        }
    }
    mat2Bitmap(env, res, bitmap);
    return bitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_mosaic(JNIEnv *env, jobject thiz, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    int src_w = src.cols;
    int src_h = src.rows;

    //省略人脸识别，只对人脸位置打马赛克，假设下面区域是人脸
    int row_s = src_h >> 2;
    int row_e = src_h * 3 / 4;
    int col_s = src_w >> 2;
    int col_e = src_w * 3 / 4;
    //马赛克大小
    int size = 30;

    for (int row = row_s; row < row_e; row += size) {
        for (int col = col_s; col < col_e; col += size) {
            int pixel = src.at<int>(row, col);
            for (int m_rows = 1; m_rows < size; m_rows++) {
                for (int m_cols = 1; m_cols < size; m_cols++) {
                    src.at<int>(row + m_rows, col + m_cols) = pixel;
                }
            }
        }
    }
    mat2Bitmap(env, src, bitmap);
    return bitmap;
}
/**
 * 毛玻璃，高斯模糊也算毛玻璃
 * 毛玻璃其实是对某个区域随机取像素
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_groundGlass(JNIEnv *env, jobject thiz, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    int src_w = src.cols;
    int src_h = src.rows;
    int size = 8;
    RNG rng(time(NULL));
    for (int row = 0; row < src_h - size; row++) {
        for (int col = 0; col < src_w - size; col++) {
            int random = rng.uniform(0, size);
            src.at<int>(row, col) = src.at<int>(row + random, col + random);
        }
    }
    mat2Bitmap(env, src, bitmap);
    return bitmap;
}
/**
 * 油画
 * 1.每个点分成n*n小块
 * 2.统计灰度等级
 * 3.选灰度等级最多的值
 * 4.找到所有像素取平均值
 *
 * 位移 > +- > *和/ > 整型 > float
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_oilPainting(JNIEnv *env, jobject thiz, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);
    Mat gray;
    cvtColor(src, gray, COLOR_BGRA2GRAY);
    Mat result(src.size(), src.type());

    int src_w = src.cols;
    int src_h = src.rows;
    int size = 8;
    for (int row = 0; row < src_h - size; row++) {
        for (int col = 0; col < src_w - size; col++) {
            //统计8*8小块里的每个灰度值
            int grayArr[8] = {0}, b_g[8] = {0}, g_g[8] = {0}, r_g[8] = {0};
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    uchar g = gray.at<uchar>(row + i, col + j);
                    uchar index = g / (254 / 7);
                    grayArr[index]++;
                    //等级的像素值之和
                    b_g[index] += src.at<Vec4b>(row + i, col + j)[0];
                    g_g[index] += src.at<Vec4b>(row + i, col + j)[1];
                    r_g[index] += src.at<Vec4b>(row + i, col + j)[2];
                }
            }
            //找最大的灰度值下标
            int max_index = 0;
            int max = grayArr[max_index];
            for (int i = 1; i < size; i++) {
                if (grayArr[max_index] < grayArr[i]) {
                    max_index = i;
                    max = grayArr[i];
                }
            }
            result.at<Vec4b>(row, col)[0] = b_g[max_index] / max;
            result.at<Vec4b>(row, col)[1] = g_g[max_index] / max;
            result.at<Vec4b>(row, col)[2] = r_g[max_index] / max;
            result.at<Vec4b>(row, col)[3] = src.at<Vec4b>(row, col)[3];
        }
    }
    mat2Bitmap(env, result, bitmap);
    return bitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_rotation(JNIEnv *env, jobject thiz, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);
    int res_w = src.rows;
    int res_h = src.cols;
    Mat res(res_h, res_w, src.type());
    for (int row = 0; row < res_h; row++) {
        for (int col = 0; col < res_w; col++) {
            int src_row = src.rows - col;
            int src_col = row;
            res.at<int>(row, col) = src.at<int>(src_row, src_col);
        }
    }
    jobject newBitmap = createBitmap(env, res_w, res_h, CV_8UC4);
    mat2Bitmap(env, res, newBitmap);
    return newBitmap;
}
/**
 * 仿射变换
 * https://www.cnblogs.com/bjxqmy/p/12337581.html
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_warpAffine(JNIEnv *env, jobject thiz, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);
    Mat res(src.size(), src.type());
    /**
     * 指定矩阵，必须传入2*3矩阵
     * [a0,a1,a2]
     * [b0,b1,b2]
     * 但要看成两个矩阵:
     * [a0,a1]    [a2]   =>  [a0,a1] * [x]+ [a2] = a0*x + a1*y + a2
     * [b0,b1]    [b2]       [b0,b1] * [y]  [b2] = b0*x + b1*y + b2
     */
//    Mat M(2, 3, CV_32FC1);
//    M.at<float>(0, 0) = 0.5;  //a0
//    M.at<float>(0, 1) = 0;  //a1
//    M.at<float>(0, 2) = 0; //a2
//    M.at<float>(1, 0) = 0;
//    M.at<float>(1, 1) = 0.5;
//    M.at<float>(1, 2) = 0;

    //系统提供了一些方法
    Point2f center(src.cols / 2, src.rows / 2);
    Mat M = getRotationMatrix2D(center, 45, 1);

    warpAffine(src, res, M, src.size());
    mat2Bitmap(env, res, bitmap);
    return bitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_resize(JNIEnv *env, jobject thiz, jobject bitmap, jint width,
                                         jint height) {
    Mat src;
    bitmap2Mat(env, src, bitmap);

    int src_w = src.cols;
    int src_h = src.rows;

    Mat res(height, width, src.type());

    for (int row = 0; row < res.rows; row++) {
        for (int col = 0; col < res.cols; col++) {
            float src_row = row * (src_h * 1.0 / height);
            float src_col = col * (src_w * 1.0 / width);
            Vec4f pixels = src.at<Vec4f>(src_row, src_col);
            res.at<Vec4f>(row, col) = pixels;
        }
    }
    jobject newBitmap = createBitmap(env, width, height, src.type());
    mat2Bitmap(env, res, newBitmap);
    return newBitmap;
}

/**
 * 重映射
 */
void remap1(Mat &src, Mat &res, Mat &matX, Mat &matY) {
    res.create(src.size(), src.type());
    int res_w = res.cols;
    int res_h = res.rows;
    for (int row = 0; row < res_h; row++) {
        for (int col = 0; col < res_w; col++) {
            int x = matX.at<float>(row, col);
            int y = matY.at<float>(row, col);
            res.at<Vec4b>(row, col) = src.at<Vec4b>(y, x);
        }
    }
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_cnting_opencv_BitmapUtil_remap(JNIEnv *env, jobject thiz, jobject bitmap) {
    Mat src;
    bitmap2Mat(env, src, bitmap);
    Mat res;
    Mat matX(src.size(), CV_32FC1);
    Mat matY(src.size(), CV_32FC1);
    for (int row = 0; row < src.rows; row++) {
        for (int col = 0; col < src.cols; col++) {
            matX.at<float>(row, col) = col;
            matY.at<float>(row, col) = src.rows - row;
        }
    }

//    remap(src, res, matX, matY, 0);
    remap1(src, res, matX, matY);  //自己实现下
    mat2Bitmap(env, res, bitmap);
    return bitmap;
}