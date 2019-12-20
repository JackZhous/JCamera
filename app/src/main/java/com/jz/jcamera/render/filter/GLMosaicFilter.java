package com.jz.jcamera.render.filter;

import android.content.Context;
import android.opengl.GLES30;

import com.jz.jcamera.R;
import com.jz.jcamera.util.OpenGLUtil;

/**
 * @author jackzhous
 * @package com.jz.jcamera.render.filter
 * @filename GLMosaicFilter
 * date on 2019/12/16 2:35 PM
 * @describe 马赛克特效
 * @email jackzhouyu@foxmail.com
 **/
public class GLMosaicFilter extends BaseFilter {


    private int perPixelWidthHandler;
    private int perPixelHeightHandler;
    private int pixelCountHandler;


    public GLMosaicFilter(Context context) {
        this(context, VERTEX_SHADER, OpenGLUtil.readRawFile(context, R.raw.fragment_mosaic_rectangle));
    }

    public GLMosaicFilter(Context context, String vertexShader, String fragmentShader) {
        super(context, vertexShader, fragmentShader);
    }


    @Override
    public void initProgramHandle() {
        super.initProgramHandle();
        if(mProgramHandle != OpenGLUtil.GL_NOT_INIT){
            perPixelWidthHandler = GLES30.glGetUniformLocation(mProgramHandle, "perPixelWidth");
            perPixelHeightHandler = GLES30.glGetUniformLocation(mProgramHandle, "perPixelHeight");
            pixelCountHandler = GLES30.glGetUniformLocation(mProgramHandle, "pixelCounts");
        }
    }

    @Override
    public void initFrameBuffer(int width, int height) {
        super.initFrameBuffer(width, height);
        if(mProgramHandle != OpenGLUtil.GL_NOT_INIT){
            setFloat(perPixelWidthHandler, 1.0f/width);
            setFloat(perPixelHeightHandler, 1.0f/height);
            setFloat(pixelCountHandler, 50);
        }
    }
}
