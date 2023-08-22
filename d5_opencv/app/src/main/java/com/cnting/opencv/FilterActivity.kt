package com.cnting.opencv

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.BitmapFactory.Options
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.cnting.opencv.databinding.ActivityFilterBinding

/**
 * Created by cnting on 2023/8/21
 * 滤镜
 */
class FilterActivity : AppCompatActivity() {
    private lateinit var binding: ActivityFilterBinding
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityFilterBinding.inflate(layoutInflater)
        setContentView(binding.root)
        val src = BitmapFactory.decodeResource(resources, R.drawable.a)
        binding.img2.setImageBitmap(BitmapUtil.gray(src))

        binding.img3.setImageBitmap(BitmapUtil.gray2(src))

        val config = Options()
        config.inPreferredConfig = Bitmap.Config.RGB_565
        val src1 = BitmapFactory.decodeResource(resources, R.drawable.a, config)
        BitmapUtil.gray3(src1)
        binding.img4.setImageBitmap(src1)
    }
}