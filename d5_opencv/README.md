* [下载地址](https://github.com/opencv/opencv/releases/tag/4.8.0)
* [配置](https://juejin.cn/post/6844903906099593229)
* [人脸识别没有face.hpp文件](https://blog.csdn.net/HikD_bn/article/details/108409007)
* [Android Studio集成OpenCV](https://www.jianshu.com/p/41d1effd630e)
* [sample导入](https://blog.csdn.net/lucky_tom/article/details/113247850)
#### 人脸识别实现思路
1. 人脸信息录入
    * 获取相机Bitmap，检测人脸（保证人脸特征信息比较精准）人脸要足够大，当前范围内只有一张人脸，，正常、眨眼睛、张嘴巴（3张人脸信息）
    * 获取到人脸必须要保存人脸特征信息，然后上传给后台（后台再次做算法优化），保存到数据库
2. 人脸的特征值匹配
    * 获取相机Bitmap，检测人脸（保证人脸特征信息比较精准）人脸要足够大，当前范围内只有一张人脸，，正常、眨眼睛、张嘴巴（3张人脸信息）
    * 从后台查询用户进行登录
3. 提取特征值和匹配人脸信息
    * 整合opencv
    * 获取相机的SurfaceView传到native层去检测
    * 检测人脸，保存人脸特征信息在本地
4.  上传到后台


#### 灰度图算法
RGB：0-255，0-255，0-255（彩色图）

灰度：R+G+B = 255 = 0.33R + 0.33G + 0.33B
但为了更好效果(适合人的眼睛)，灰度值 = 0.2989R + 0.5870G + 0.114B


灰度图所含有的信息量（取值范围0~255）不高，而彩色图的取值范围比较高（255 * 255 * 255）
