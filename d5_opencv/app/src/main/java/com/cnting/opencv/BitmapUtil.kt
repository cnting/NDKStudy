package com.cnting.opencv

import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.ColorMatrix
import android.graphics.ColorMatrixColorFilter
import android.graphics.Paint

object BitmapUtil {
    /**
     * 使用ColorMatrix操作矩阵
     */
    fun gray(src: Bitmap): Bitmap {
        val dst = Bitmap.createBitmap(src.width, src.height, src.config)
        val canvas = Canvas(dst)
        val paint = Paint()
        paint.isDither = true
        paint.isAntiAlias = true

        val colorMatrix = ColorMatrix()
        colorMatrix.setSaturation(0f)
        paint.colorFilter = ColorMatrixColorFilter(colorMatrix)

        canvas.drawBitmap(src, 0f, 0f, paint)

        return dst
    }

    /**
     * Java层操作像素
     */
    fun gray2(src: Bitmap): Bitmap {
        val dst = Bitmap.createBitmap(src.width, src.height, src.config)
        val pixels = IntArray(src.width * src.height)
        src.getPixels(pixels, 0, src.width, 0, 0, src.width, src.height)

        (pixels.indices).forEach {
            val pixel = pixels[it]
            //ARGB
            val a: Int = (pixel.shr(24)) and 0xff  //it>>24 & 0xff，取前8位
            val r: Int = (pixel.shr(16)) and 0xff
            val g: Int = (pixel.shr(8)) and 0xff
            val b: Int = pixel and 0xff
            val gray: Int = (0.213f * r + 0.715 * g + 0.072f * b).toInt()

            //(a<<24) | (gray<<16) | (gray<<8) | (gray)
            pixels[it] = (a shl 24) or (gray shl 16) or (gray shl 8) or gray

        }
        dst.setPixels(pixels, 0, src.width, 0, 0, src.width, src.height)
        return dst

    }

    /**
     * 在native层操作Bitmap
     */
    external fun gray3(src: Bitmap): Int

    /**
     * 逆世界
     */
    external fun inverseWorld(bitmap: Bitmap): Bitmap

    /**
     * 镜像
     */
    external fun mirror(bitmap: Bitmap): Bitmap

    /**
     * 浮雕
     */
    external fun anaglyph(bitmap: Bitmap): Bitmap

    /**
     * 马赛克
     */
    external fun mosaic(bitmap: Bitmap): Bitmap

    /**
     * 毛玻璃
     */
    external fun groundGlass(bitmap: Bitmap): Bitmap

    /**
     * 油画
     */
    external fun oilPainting(bitmap: Bitmap): Bitmap

    /**
     * 旋转
     */
    external fun rotation(bitmap: Bitmap): Bitmap

    /**
     * 仿射变换，实现一些简单的重映射
     */
    external fun warpAffine(bitmap: Bitmap): Bitmap

    /**
     * 缩放
     */
    external fun resize(bitmap: Bitmap, width: Int, height: Int): Bitmap

    /**
     * 重映射
     */
    external fun remap(bitmap: Bitmap): Bitmap


}
