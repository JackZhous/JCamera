package com.jz.jcamera.render.filter;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Path;
import android.opengl.GLES30;
import android.opengl.GLUtils;

import com.jz.jcamera.R;
import com.jz.jcamera.util.JLog;
import com.jz.jcamera.util.OpenGLUtil;

/**
 * @author jackzhous
 * @package com.jz.jcamera.render.filter
 * @filename GLbeautifyFilter
 * date on 2019/12/18 10:14 AM
 * @describe
 * 人脸美颜
 * @email jackzhouyu@foxmail.com
 **/
public class GLbeautifyFilter extends BaseFilter {
    private int stepHandler;
    //美颜参数
    private int paramsHandler;

    private int testImageHandler;
    private int testTexture;

    public GLbeautifyFilter(Context context) {
        this(context, VERTEX_SHADER, OpenGLUtil.readRawFile(context, R.raw.fragment_beautify));
    }

    public GLbeautifyFilter(Context context, String vertexShader, String fragmentShader) {
        super(context, vertexShader, fragmentShader);
    }

    @Override
    public void initProgramHandle() {
        super.initProgramHandle();
        if(mProgramHandle != OpenGLUtil.GL_NOT_INIT){
            stepHandler = GLES30.glGetUniformLocation(mProgramHandle, "singleStepOffset");
            paramsHandler = GLES30.glGetUniformLocation(mProgramHandle, "params");
//            testImageHandler = GLES30.glGetUniformLocation(mProgramHandle, "bitmapInputTexture");
//            init();
        }
    }

    private void init(){
        Bitmap bitmap = BitmapFactory.decodeResource(mContext.getResources(), R.drawable.chicken);
        testTexture = OpenGLUtil.createTexture(GLES30.GL_TEXTURE_2D);
        GLUtils.texImage2D(GLES30.GL_TEXTURE_2D, 0, bitmap, 0);
    }

    @Override
    public void onDrawFrameBegin() {
        super.onDrawFrameBegin();
//        OpenGLUtil.bindTexture(testImageHandler, testTexture, 1);
    }

    @Override
    public void release() {
        super.release();
//        if(testTexture != OpenGLUtil.GL_NOT_TEXTURE){
//            GLES30.glDeleteTextures(1, new int[]{testTexture}, 0);
//        }
    }

    @Override
    public void onDisplaySizeChanged(int width, int height) {
        super.onDisplaySizeChanged(width, height);
        if(stepHandler != OpenGLUtil.GL_NOT_INIT){
            setFloatVec2(stepHandler, new float[]{2.0f/width, 2.0f/height});
        }

        if(paramsHandler != OpenGLUtil.GL_NOT_INIT){
            setFloat(paramsHandler, 1f);
        }
    }
}
