### 音视频同步
[解析](https://ffmpeg.xianwaizhiyin.net/ffplay/basic_sync.html)
思路：以声音为基准，视频解码慢点来同步声音

一些概念：
* pts：显示时间戳，用来告诉播放器该在什么时候显示这一帧数据。单位是不确定的
* time_base：时间度量单位（时间基）