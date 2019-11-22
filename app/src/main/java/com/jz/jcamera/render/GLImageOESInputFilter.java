package com.jz.jcamera.render;

import android.content.Context;
import android.opengl.GLES11Ext;
import android.opengl.GLES30;

import com.jz.jcamera.R;
import com.jz.jcamera.util.OpenGLUtil;

/**
 * @author jackzhous
 * @package com.jz.jcamera.render
 * @filename GLImageOESInputFilter
 * date on 2019/11/22 11:16 AM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class GLImageOESInputFilter extends BaseFilter{

    private int mTransformMatrixHandle;
    private float[] mTransformMatrix;

    public GLImageOESInputFilter(Context context) {
        super(context, OpenGLUtil.readRawFile(context, R.raw.vertex_shader),
                OpenGLUtil.readRawFile(context, R.raw.fragment_shader));
    }

    @Override
    public void initProgramHandle() {
        super.initProgramHandle();
        mTransformMatrixHandle = GLES30.glGetUniformLocation(mProgramHandle, "transformMatrix");
    }


    @Override
    public int getTextureType() {
        return GLES11Ext.GL_TEXTURE_EXTERNAL_OES;
    }


    @Override
    public void onDrawFrameBegin() {
        super.onDrawFrameBegin();
        GLES30.glUniformMatrix4fv(mTransformMatrixHandle, 1 , false, mTransformMatrix, 0);
    }

    public void setmTransformMatrix(float[] mTransformMatrix) {
        this.mTransformMatrix = mTransformMatrix;
    }
}
