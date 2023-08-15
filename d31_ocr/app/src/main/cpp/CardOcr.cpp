//
// Created by cnting on 2023/8/15.
//

#include "CardOcr.h"
#include "vector"
#include "android/log.h"

using namespace std;

int co1::find_card_area(const cv::Mat &mat, cv::Rect &area) {
    //首先降噪(高斯模糊)
    Mat blur;
    GaussianBlur(mat, blur, Size(5, 5), BORDER_DEFAULT, BORDER_DEFAULT);

    //边缘梯度增强（保存图片）
    Mat gard_x, gard_y;
    Scharr(blur, gard_x, CV_32F, 1, 0);
    Scharr(blur, gard_y, CV_32F, 0, 1);
    Mat abs_gard_x, abs_gard_y;
    convertScaleAbs(gard_x, abs_gard_x);
    convertScaleAbs(gard_y, abs_gard_y);
    Mat gard;
    addWeighted(abs_gard_x, 0.5, abs_gard_y, 0.5, 0, gard);
    imwrite("/storage/emulated/0/Android/data/com.cnting.ocr/gard.jpg", gard);

    //二值化，筛选轮廓
    Mat gray;
    cvtColor(gard, gray, COLOR_BGRA2GRAY);
    imwrite("/storage/emulated/0/Android/data/com.cnting.ocr/gray.jpg", gray);

    Mat binary;
    threshold(gray, binary, 40, 255, THRESH_BINARY);
    imwrite("/storage/emulated/0/Android/data/com.cnting.ocr/binary.jpg", binary);

    vector<vector<Point>> contours;
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    for (int i = 0; i < contours.size(); i++) {
        Rect rect = boundingRect(contours[i]);
        //过滤轮廓，银行卡区域大宽高必须大于图片的一半
        if (rect.width > mat.cols / 2 && rect.width != mat.cols && rect.height > mat.rows / 2) {
            area = rect;
            break;
        }
    }

    //mat提供了释放函数，一般要调用
    blur.release();
    gard_x.release();
    gard_y.release();
    abs_gard_x.release();
    abs_gard_y.release();
    binary.release();

    return 0;
}

int co1::find_card_number_area(const cv::Mat &mat, cv::Rect &area) {
    //有两种方式：
    //一种是精确截取，找到银联区域，通过大小比例精确的截取
    //一种是粗略截取，截取高度1/2 ~ 3/4，宽度1/12 ~ 10/12（下面代码采用这种方式）
    //万一找不到，可以手动输入和修改
    area.x = mat.cols / 12;
    area.y = mat.rows / 2;
    area.width = mat.cols * 5 / 6;
    area.height = mat.rows / 4;
    return 0;
}

int co1::find_card_numbers(const cv::Mat &mat, vector<cv::Mat> numbers) {
    //二值化，灰度处理
    Mat gray;
    cvtColor(mat, gray, COLOR_BGRA2GRAY);
    Mat binary;
    threshold(gray, binary, 39, 255, THRESH_BINARY);
    imwrite("/storage/emulated/0/Android/data/com.cnting.ocr/card_number_binary.jpg", binary);

    //降噪过滤
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(binary, binary, MORPH_CLOSE, kernel);

    //去掉干扰过滤填充
    //查找轮廓，要先变成黑色背景，白色数字，所以要先取反
    Mat binary_not = binary.clone();
    bitwise_not(binary_not, binary_not);
    imwrite("/storage/emulated/0/Android/data/com.cnting.ocr/card_number_binary_not.jpg",
            binary_not);

    vector<vector<Point>> contours;
    findContours(binary_not, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    int mat_area = mat.rows * mat.cols;
    int min_height = mat.rows / 4;
    for (int i = 0; i < contours.size(); i++) {
        Rect rect = boundingRect(contours[i]);
        //面积太小的过滤
        int area = rect.width * rect.height;
        if (area < mat_area / 200) {
            __android_log_print(ANDROID_LOG_ERROR, "TAG", "过滤小的");
            //小面积填充为白色背景
            drawContours(binary, contours, i, Scalar(255), -1);
        } else if (rect.height < min_height) {
            drawContours(binary, contours, i, Scalar(255), -1);
            __android_log_print(ANDROID_LOG_ERROR, "TAG", "过滤矮的");
        }
    }
    imwrite("/storage/emulated/0/Android/data/com.cnting.ocr/card_number_binary_noise.jpg",
            binary);

    //截取每个数字的轮廓
    //目前binary是没有噪音的，binary_not是有噪音的，所以要再次取反
    binary.copyTo(binary_not);
    bitwise_not(binary_not, binary_not);

    //先把rect存起来
    contours.clear();
    findContours(binary_not, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    Rect rects[contours.size()];

    //白色图片，单颜色
    Mat contours_mat(binary.size(), CV_8UC1, Scalar(255));
    //判断粘连字符
    int min_w = mat.cols;
    for (int i = 0; i < contours.size(); i++) {
        rects[i] = boundingRect(contours[i]);
        drawContours(contours_mat, contours, i, Scalar(0), 1);
        min_w = min(rects[i].width, min_w);
    }

    imwrite("/storage/emulated/0/Android/data/com.cnting.ocr/card_number_contours_mat.jpg",
            contours_mat);

    //排序
    for (int i = 0; i < contours.size() - 1; i++) {
        for (int j = 0; j < contours.size() - i - 1; j++) {
            if (rects[j].x > rects[j + 1].x) {
                swap(rects[j], rects[j + 1]);
            }
        }
    }
    //裁剪

    numbers.clear();
    for (int i = 0; i < contours.size(); i++) {
        if (rects[i].width >= min_w * 2) {
            //处理粘连字符
            Mat mat(contours_mat, rects[i]);
            int clos_pos = find_split_clos_pos(mat);
            Rect rect_left(0, 0, clos_pos - 1, mat.rows);
            numbers.push_back(Mat(mat, rect_left));

            Rect rect_right(clos_pos, 0, mat.cols, mat.rows);
            numbers.push_back(Mat(mat, rect_right));

        } else {
            Mat number(contours_mat, rects[i]);
            numbers.push_back(number);

            char name[70];
            sprintf(name, "/storage/emulated/0/Android/data/com.cnting.ocr/card_number_%d.jpg", i);
            imwrite(name,
                    number);
        }

    }

    gray.release();
    binary.release();
    binary_not.release();
    contours_mat.release();
    return 0;
}

int co1::find_split_clos_pos(cv::Mat &mat) {
    //对中心位置的左右1/4扫描，记录最少的黑色像素点的这一列的位置，就当做字符串的粘连位置
    int mx = mat.cols / 2;
    int height = mat.rows;
    //围绕中心左右扫描1/4
    int start_x = mx - mx / 2;
    int end_x = mx + mx / 2;
    //字符的粘连位置
    int clos_pos = mx;
    //获取像素值
    int c = 0;
    //最小的像素值
    int min_h_p = mat.rows;
    for (int col = start_x; col < end_x; col++) {
        int total = 0;
        for (int row = 0; row < height; row++) {
            //获取像素点
            c = mat.at<Vec3b>(row, col)[0]; //单通道
            if (c == 0) { //0表示黑色
                total++;
            }
        }
        if (total < min_h_p) {
            min_h_p = total;
            clos_pos = col;
        }
    }
    return clos_pos;
}
