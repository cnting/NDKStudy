package com.cnting.livepush.camera;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.Matrix;


import com.cnting.livepush.R;
import com.cnting.livepush.egl.XYEGLSurfaceView;
import com.cnting.livepush.egl.XYShaderUtil;
import com.cnting.livepush.util.DisplayUtil;
import com.cnting.livepush.util.LogUtil;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

/**
 * @author liuml
 * @explain
 * @time 2018/12/7 15:54
 */
public class XYCameraRender implements XYEGLSurfaceView.XYGLRender, SurfaceTexture.OnFrameAvailableListener {

    private Context context;

    private float[] vertexData = {
            -1f, -1f,
            1f, -1f,
            -1f, 1f,
            1f, 1f

    };

    private float[] fragmentData = {
            0f, 1f,
            1f, 1f,
            0f, 0f,
            1f, 0f


    };

    private FloatBuffer vertexBuffer;
    private FloatBuffer fragmentBuffer;

    private int program;
    private int vPosition;
    private int fPosition;
    private int vboId;
    private int fboId;

    private int fboTextureid;
    private int cameraTextureid;//摄像头纹理

    private int screenW = 1080;
    private int screenH = 1920;

    //实际渲染的大小
    private int width;
    private int height;

    private SurfaceTexture surfaceTexture;
    private XYCameraFboRender xyCameraFboRender;

    private int umatrix;
    private float[] matrix = new float[16];


    private OnSurfaceCreateListener onSurfaceCreateListener;

    public XYCameraRender(Context context) {
        this.context = context;

        screenW = DisplayUtil.getScreenWidth(context);
        screenH = DisplayUtil.getScreenHeight(context);

        xyCameraFboRender = new XYCameraFboRender(context);

        vertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(vertexData);
        vertexBuffer.position(0);

        fragmentBuffer = ByteBuffer.allocateDirect(fragmentData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(fragmentData);
        fragmentBuffer.position(0);


    }

    public void setOnSurfaceCreateListener(OnSurfaceCreateListener onSurfaceCreateListener) {
        this.onSurfaceCreateListener = onSurfaceCreateListener;
    }

    @Override
    public void onSurfaceCreated() {

        xyCameraFboRender.onCreate();
        //获取顶点以及片元属性
        String vertexSource = XYShaderUtil.getRawResource(context, R.raw.vertex_shader);
        String fragmentSource = XYShaderUtil.getRawResource(context, R.raw.fragment_shader);

        program = XYShaderUtil.createProgram(vertexSource, fragmentSource);

        vPosition = GLES20.glGetAttribLocation(program, "v_Position");
        fPosition = GLES20.glGetAttribLocation(program, "f_Position");
        umatrix = GLES20.glGetUniformLocation(program, "u_Matrix");

        //VBO
        int[] vbos = new int[1];
//        1、创建VBO
        GLES20.glGenBuffers(1, vbos, 0);
        vboId = vbos[0];
        //2.绑定VBO
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, vboId);
//        GLES20.glBufferData(GLES20.GL_ARRAY_BUFFER, vertexData.length * 4, null, GLES20.GL_STATIC_DRAW);

        //3. 分配VBO需要的缓冲大小
        GLES20.glBufferData(GLES20.GL_ARRAY_BUFFER, vertexData.length * 4 + fragmentData.length * 4, null, GLES20.GL_STATIC_DRAW);

        //4,为VBO设置顶点数据的值(这里想象内存区域 先偏移0用于存储顶点坐标,再偏移顶点坐标的大小,用于存储片元坐标) 大小是顶点坐标加上片元坐标大小
        GLES20.glBufferSubData(GLES20.GL_ARRAY_BUFFER, 0, vertexData.length * 4, vertexBuffer);
        GLES20.glBufferSubData(GLES20.GL_ARRAY_BUFFER, vertexData.length * 4, fragmentData.length * 4, fragmentBuffer);
//        5、解绑VBO
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, 0);


        //fbo
        int[] fbos = new int[1];
        GLES20.glGenBuffers(1, fbos, 0);
        fboId = fbos[0];

        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, fboId);


        int[] textureIds = new int[1];
        GLES20.glGenTextures(1, textureIds, 0);
        fboTextureid = textureIds[0];

        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, fboTextureid);
//        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
//        GLES20.glUniform1i(sampler, 0);

        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);

        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);

        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, screenW, screenH, 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, null);
        GLES20.glFramebufferTexture2D(GLES20.GL_FRAMEBUFFER, GLES20.GL_COLOR_ATTACHMENT0, GLES20.GL_TEXTURE_2D, fboTextureid, 0);//把纹理绑定到FBO上

        if (GLES20.glCheckFramebufferStatus(GLES20.GL_FRAMEBUFFER) != GLES20.GL_FRAMEBUFFER_COMPLETE) {
            LogUtil.e("fbo wrong");
        } else {
            LogUtil.e("fbo success");
        }


//        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);

        //扩展纹理  eos
        int[] textureidsEos = new int[1];
        GLES20.glGenTextures(1, textureidsEos, 0);
        cameraTextureid = textureidsEos[0];
//

        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, cameraTextureid);

        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);

        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);

        surfaceTexture = new SurfaceTexture(cameraTextureid);
        surfaceTexture.setOnFrameAvailableListener(this);

        if (onSurfaceCreateListener != null) {
            onSurfaceCreateListener.onSurfaceCreate(surfaceTexture,fboTextureid);
        }
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, 0);

    }

    /**
     * 重置矩阵
     */
    public void resetMatrix(){
        Matrix.setIdentityM(matrix, 0);
    }

    /**
     * 设置角度
     * @param angle
     * @param x
     * @param y
     * @param z
     */
    public void setAngle(float angle, float x, float y, float z) {
        //旋转
        Matrix.rotateM(matrix, 0, angle, x, y, z);
    }

    @Override
    public void onSurfaceChanged(int width, int height) {
        this.width = width;
        this.height = height;
//        xyCameraFboRender.onChange(width, height);
//       GLES20.glViewport(0, 0, width, height);

//        //旋转
//        Matrix.rotateM(matrix, 0, 90, 0, 0, 1);//沿着z轴 90度
//        Matrix.rotateM(matrix, 0, 180, 1, 0, 0);//沿着x周180度

    }

    @Override
    public void onDrawFrame() {

        surfaceTexture.updateTexImage();

        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(1f, 0f, 0f, 1f);

        GLES20.glUseProgram(program);

        GLES20.glViewport(0, 0, screenW, screenH);
        //使用program后调用矩阵
        GLES20.glUniformMatrix4fv(umatrix, 1, false, matrix, 0);


        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, fboId);
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, vboId);


        GLES20.glEnableVertexAttribArray(vPosition);
        GLES20.glVertexAttribPointer(vPosition, 2, GLES20.GL_FLOAT, false, 8,
                0);

        GLES20.glEnableVertexAttribArray(fPosition);
        GLES20.glVertexAttribPointer(fPosition, 2, GLES20.GL_FLOAT, false, 8,
                vertexData.length * 4);


        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, 0);

        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);

        xyCameraFboRender.onChange(width, height);//
        //绘制
        xyCameraFboRender.onDraw(fboTextureid);

    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        //预览的 当有数据会回调这个方法

    }

    public interface OnSurfaceCreateListener {
        void onSurfaceCreate(SurfaceTexture surfaceTexture,int textureId);
    }

    public int getFboTextureid() {
        return fboTextureid;
    }
}
