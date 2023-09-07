package com.cnting.ffmpeg

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.widget.TextView
import com.cnting.ffmpeg.databinding.ActivityMainBinding
import com.cnting.ffmpeg.media.CTPlayer
import com.cnting.ffmpeg.media.listener.MediaErrorListener
import java.io.File
import kotlin.concurrent.thread

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var player: CTPlayer


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val file = File(getExternalFilesDir(null), "Utakata.mp3")
        val url = "http://music.163.com/song/media/outer/url?id=20110049.mp3"

        Log.d("===>", file.absolutePath)

        player = CTPlayer()
        player.setDataSource(file.absolutePath)
        player.setErrorListener(object : MediaErrorListener {
            override fun onError(code: Int, msg: String) {
                Log.e("===>", "code:$code,msg:$msg")
            }
        })

        binding.sampleText.setOnClickListener {
            player.play()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        player.release()
    }

}