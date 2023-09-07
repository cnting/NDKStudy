package com.cnting.opensles

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import com.cnting.opensles.databinding.ActivityMainBinding
import java.io.File

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        //生成pcm命令：
        //ffmpeg -i input.mp4 -ar 44100 -ac 2 -f s16le output.pcm
        val file = File(getExternalFilesDir(null), "output.pcm")
        playPCM(file.absolutePath)
    }

    private external fun playPCM(path: String)


    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}