package com.jz.jcamera.render;

import android.content.Context;
import android.opengl.GLES30;

import com.jz.jcamera.R;
import com.jz.jcamera.util.OpenGLUtil;

/**
 * @author jackzhous
 * @package com.jz.jcamera.render
 * @filename GLSplitScreenGuassFilter
 * date on 2019/12/10 5:06 PM
 * @describe 上中下三屏幕分屏
 * @email jackzhouyu@foxmail.com
 **/
public class GLSplitScreenGuassFilter extends BaseFilter{

    private int XLenHandler;
    private int YLenHandler;
    private int XStepHandler;
    private int YStepHandler;
    public GLSplitScreenGuassFilter(Context context) {
        this(context, OpenGLUtil.readRawFile(context, R.raw.vertex_split_screen_guass), OpenGLUtil.readRawFile(context, R.raw.fragment_split_screen_guass));
    }

    public GLSplitScreenGuassFilter(Context context, String vertexShader, String fragmentShader) {
        super(context, vertexShader, fragmentShader);
    }


    @Override
    public void initProgramHandle() {
        super.initProgramHandle();
        if(mProgramHandle != OpenGLUtil.GL_NOT_INIT){
            XLenHandler = GLES30.glGetUniformLocation(mProgramHandle, "xLen");
            YLenHandler = GLES30.glGetUniformLocation(mProgramHandle, "yLen");
            XStepHandler = GLES30.glGetUniformLocation(mProgramHandle, "xStep");
            YStepHandler = GLES30.glGetUniformLocation(mProgramHandle, "yStep");
        }
    }


    @Override
    public void onDisplaySizeChanged(int width, int height) {
        super.onDisplaySizeChanged(width, height);
        setFloat(XStepHandler, 12.0f/width);
        setFloat(YStepHandler, 12.0f/height);
    }
}
