#### 银行卡识别
步骤：
###### 1. 截取到银行卡区域
思路：
1.轮廓增强（梯度增强）
2.进行二值化轮廓过滤
3.截取卡号区域
  方式一：找到银联区域，按比例找到卡号位置(精确截取)
  方式二：直接截取卡号(粗略截取，大概位置在卡的1/2至3/4范围)
