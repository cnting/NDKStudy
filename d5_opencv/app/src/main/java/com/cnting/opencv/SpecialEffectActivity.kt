package com.cnting.opencv

import android.graphics.BitmapFactory
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.cnting.opencv.databinding.ActivitySpecialEffectBinding

/**
 * Created by cnting on 2023/8/26
 *
 */
class SpecialEffectActivity : AppCompatActivity() {
    private lateinit var binding: ActivitySpecialEffectBinding
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivitySpecialEffectBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.inverseWorldBtn.setOnClickListener {
            val bitmap = BitmapFactory.decodeResource(resources, R.drawable.a)
            val res = BitmapUtil.inverseWorld(bitmap)
            binding.specialEffectImg.setImageBitmap(res)
        }
        binding.mirrorBtn.setOnClickListener {
            val bitmap = BitmapFactory.decodeResource(resources, R.drawable.a)
            val res = BitmapUtil.mirror(bitmap)
            binding.specialEffectImg.setImageBitmap(res)
        }
        binding.anaglyphBtn.setOnClickListener {
            val bitmap = BitmapFactory.decodeResource(resources, R.drawable.a)
            val res = BitmapUtil.anaglyph(bitmap)
            binding.specialEffectImg.setImageBitmap(res)
        }
        binding.mosaicBtn.setOnClickListener {
            val bitmap = BitmapFactory.decodeResource(resources, R.drawable.a)
            val res = BitmapUtil.mosaic(bitmap)
            binding.specialEffectImg.setImageBitmap(res)
        }
        binding.groundGlassBtn.setOnClickListener {
            val bitmap = BitmapFactory.decodeResource(resources, R.drawable.a)
            val res = BitmapUtil.groundGlass(bitmap)
            binding.specialEffectImg.setImageBitmap(res)
        }
        binding.oilPaintingBtn.setOnClickListener {
            val bitmap = BitmapFactory.decodeResource(resources, R.drawable.a)
            val res = BitmapUtil.oilPainting(bitmap)
            binding.specialEffectImg.setImageBitmap(res)
        }
    }
}