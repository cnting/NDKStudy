package com.cnting.ocr

import android.graphics.BitmapFactory
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.widget.TextView
import com.cnting.ocr.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private val bankCardOcr: BankCardOcr by lazy { BankCardOcr() }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val bitmap = BitmapFactory.decodeResource(resources, R.drawable.card1)
        binding.cardImg.setImageBitmap(bitmap)

        binding.cardBtn.setOnClickListener {
            binding.cardTv.text = bankCardOcr.cardOcr(bitmap)
        }
    }


}