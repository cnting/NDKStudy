package com.cnting.ffmpeg

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.widget.TextView
import com.cnting.ffmpeg.databinding.ActivityMainBinding
import java.io.File
import kotlin.concurrent.thread

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val inFile = File(getExternalFilesDir("video"), "test.mp4")
        val outFile = File(getExternalFilesDir("video"), "out.mp4")

        val callback = object : VideoCompress.CompressCallback {
            override fun onCompress(current: Int, total: Int) {
                Log.e("===>", "压缩进度：${current}/${total}")
            }
        }
        val videoCompress = VideoCompress()
        val command =
            arrayOf("ffmpeg", "-i", inFile.absolutePath, "-b:v", "1024k", outFile.absolutePath)

        binding.sampleText.setOnClickListener {
            thread { videoCompress.compressVideo(command, callback) }
        }
    }

}