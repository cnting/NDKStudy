package com.cnting.opencv

import android.graphics.BitmapFactory
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.cnting.opencv.databinding.ActivityTransformBinding

/**
 * Created by cnting on 2023/8/28
 *
 */
class TransformActivity : AppCompatActivity() {
    lateinit var binding: ActivityTransformBinding
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityTransformBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.rotationBtn.setOnClickListener {
            val bitmap = BitmapFactory.decodeResource(resources, R.drawable.a)
            val res = BitmapUtil.rotation(bitmap)
            binding.specialEffectImg.setImageBitmap(res)
        }
        binding.warpAffineBtn.setOnClickListener {
            val bitmap = BitmapFactory.decodeResource(resources, R.drawable.a)
            val res = BitmapUtil.warpAffine(bitmap)
            binding.specialEffectImg.setImageBitmap(res)
        }
        binding.resizeBtn.setOnClickListener {
            val bitmap = BitmapFactory.decodeResource(resources, R.drawable.a)
//            val res = BitmapUtil.resize(bitmap, bitmap.width / 2, bitmap.height / 2)
            val res = BitmapUtil.resize(bitmap, bitmap.width * 2, bitmap.height * 2)
            binding.specialEffectImg.setImageBitmap(res)
        }
        binding.remapBtn.setOnClickListener {
            val bitmap = BitmapFactory.decodeResource(resources, R.drawable.a)
            val res = BitmapUtil.remap(bitmap)
            binding.specialEffectImg.setImageBitmap(res)
        }
    }
}