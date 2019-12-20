package com.jz.jcamera.render.filter;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES30;
import android.opengl.GLUtils;

import com.jz.jcamera.R;
import com.jz.jcamera.util.OpenGLUtil;

/**
 * @author jackzhous
 * @package com.jz.jcamera.render.filter
 * @filename GLImageBorderFilter
 * date on 2019/12/17 3:03 PM
 * @describe
 * 边框滤镜，为图像加一层边框
 * @email jackzhouyu@foxmail.com
 **/
public class GLImageBorderFilter extends BaseFilter {
    //边框采样器句柄
    private int borderHandler;
    //边框纹理
    private int borderTexture;

    public GLImageBorderFilter(Context context) {
        this(context, VERTEX_SHADER, OpenGLUtil.readRawFile(context, R.raw.fragment_border_filter));
    }

    public GLImageBorderFilter(Context context, String vertexShader, String fragmentShader) {
        super(context, vertexShader, fragmentShader);
    }

    @Override
    public void initProgramHandle() {
        super.initProgramHandle();
        if(mProgramHandle != OpenGLUtil.GL_NOT_INIT){
            borderHandler = GLES30.glGetUniformLocation(mProgramHandle, "bitmapTexture");
            Bitmap bitmap = BitmapFactory.decodeResource(mContext.getResources(), R.drawable.filter_black_bg);
            initBorderTexture(bitmap);
        }
    }


    /**
     * 初始化边框纹理
     * @param bitmap
     */
    private void initBorderTexture(Bitmap bitmap){
        borderTexture = OpenGLUtil.createTexture(GLES30.GL_TEXTURE_2D);
        GLUtils.texImage2D(GLES30.GL_TEXTURE_2D, 0, bitmap, 0);
    }

    @Override
    public void onDrawFrameBegin() {
        super.onDrawFrameBegin();
        OpenGLUtil.bindTexture(borderHandler, borderTexture, 1);
    }

    @Override
    public void release() {
        super.release();
        if(borderTexture != OpenGLUtil.GL_NOT_TEXTURE){
            GLES30.glDeleteTextures(1, new int[]{borderTexture}, 0);
        }
    }
}
