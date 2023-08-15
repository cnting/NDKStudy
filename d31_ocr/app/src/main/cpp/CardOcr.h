//
// Created by cnting on 2023/8/15.
//


#ifndef D31_OCR_CARDOCR_H
#define D31_OCR_CARDOCR_H

#include "opencv2/opencv.hpp"
#include "vector"

using namespace cv;
using namespace std;

namespace co1 {
    /**
     * 找银行卡区域
     * @param mat
     * @param area 银行卡区域
     * @return 是否成功，0成功，其他失败
     */
    int find_card_area(const Mat &mat, Rect& area);

    /**
     * 通过银行卡区域截取卡号区域
     * @param mat
     * @param rect 卡号区域
     * @return 是否成功，0成功，其他失败
     */
    int find_card_number_area(const Mat&mat, Rect&area);

    /**
     * 找到所有数字
     * @param mat 银行卡号区域
     * @param numbers 存放所有数字
     * @return 是否成功，0成功，其他失败
     */
    int find_card_numbers(const Mat&mat,vector<Mat> numbers);

    /**
     * 字符串粘连处理
     * @param mat
     * @return 粘连的那一列
     */
    int find_split_clos_pos(Mat &mat);

}
namespace co2 {

}
class CardOcr {

};


#endif //D31_OCR_CARDOCR_H
