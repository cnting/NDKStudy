package com.cnting.parcel

import android.os.Parcel

/**
 * Created by cnting on 2023/8/11
 *
 */
class Parcel {
    private var mNativePtr: Long = 0

    init {
        System.loadLibrary("parcel")
        mNativePtr = nativeCreate()
    }

    fun writeInt(value: Int) {
        nativeWriteInt(mNativePtr, value)
    }

    fun setDataPosition(pos: Int) {
        nativeSetDataPosition(mNativePtr, pos)
    }

    fun readInt(): Int {
        return nativeReadInt(mNativePtr)
    }

    private external fun nativeSetDataPosition(nativePtr: Long, pos: Int)
    private external fun nativeWriteInt(mNativePtr: Long, value: Int)
    private external fun nativeReadInt(nativePtr: Long): Int
    private external fun nativeCreate(): Long

}