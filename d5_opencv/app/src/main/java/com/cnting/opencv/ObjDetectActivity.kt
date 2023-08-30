package com.cnting.opencv

import android.graphics.BitmapFactory
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.cnting.opencv.databinding.ActivityDetectBinding
import com.cnting.opencv.databinding.ActivityTransformBinding

/**
 * Created by cnting on 2023/8/28
 * 物体检测
 */
class ObjDetectActivity : AppCompatActivity() {
    lateinit var binding: ActivityDetectBinding
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityDetectBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.peoplesBtn.setOnClickListener {
            val bitmap = BitmapFactory.decodeResource(resources, R.drawable.peoples)
            val res = ObjDetectUtil.hogBitmap(bitmap)
            binding.specialEffectImg.setImageBitmap(res)
        }
        binding.lbpBtn.setOnClickListener {
            val bitmap = BitmapFactory.decodeResource(resources, R.drawable.a)
            val res = ObjDetectUtil.lbpBitmap(bitmap)
            binding.specialEffectImg.setImageBitmap(res)
        }
    }
}