package com.jz.jcamera.render.filter;

import android.content.Context;
import android.opengl.GLES20;

import com.jz.jcamera.R;
import com.jz.jcamera.render.filter.BaseFilter;
import com.jz.jcamera.util.OpenGLUtil;

/**
 * @author jackzhous
 * @package com.jz.jcamera.render
 * @filename GLImageGaussPassFilter
 * date on 2019/12/9 10:45 AM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class GLImageGaussPassFilter extends BaseFilter {

    protected float mBlurSize = 1f;

    private int mTexelWidthOffsetHandle;
    private int mTexelHeightOffsetHandle;

    private float mTexelWidth;
    private float mTexelHeight;

    public GLImageGaussPassFilter(Context context) {
        this(context, OpenGLUtil.readRawFile(context, R.raw.vertex_guass),
                OpenGLUtil.readRawFile(context, R.raw.fragment_guass));
    }

    public GLImageGaussPassFilter(Context context, String vertexShader, String fragmentShader) {
        super(context, vertexShader, fragmentShader);
    }

    @Override
    public void initProgramHandle() {
        super.initProgramHandle();
        mTexelWidthOffsetHandle = GLES20.glGetUniformLocation(mProgramHandle, "texelWidthOffset");
        mTexelHeightOffsetHandle = GLES20.glGetUniformLocation(mProgramHandle, "texelHeightOffset");
    }

    /**
     * 设置模糊半径大小，默认为1.0f
     * @param blurSize
     */
    public void setBlurSize(float blurSize) {
        mBlurSize = blurSize;
    }

    /**
     * 设置高斯模糊的宽高
     * @param width
     * @param height
     */
    public void setTexelOffsetSize(float width, float height) {
        mTexelWidth = width;
        mTexelHeight = height;
        if (mTexelWidth != 0) {
            setFloat(mTexelWidthOffsetHandle, mBlurSize / mTexelWidth);
        } else {
            setFloat(mTexelWidthOffsetHandle, 0.0f);
        }
        if (mTexelHeight != 0) {
            setFloat(mTexelHeightOffsetHandle, mBlurSize / mTexelHeight);
        } else {
            setFloat(mTexelHeightOffsetHandle, 0.0f);
        }
    }
}
