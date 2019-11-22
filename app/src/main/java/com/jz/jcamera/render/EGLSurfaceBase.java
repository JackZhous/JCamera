package com.jz.jcamera.render;

import android.opengl.EGL14;
import android.opengl.EGLSurface;
import android.opengl.GLES30;

import com.jz.jcamera.opengl.EGLHelper;
import com.jz.jcamera.util.JLog;
import com.jz.jcamera.util.OpenGLUtil;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * @author jackzhous
 * @package com.jz.jcamera.render
 * @filename EGLSurfaceBase
 * date on 2019/11/22 2:55 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class EGLSurfaceBase {
    protected EGLHelper eglHelper;
    private EGLSurface mEGLSurface = EGL14.EGL_NO_SURFACE;
    private int mWidth = -1;
    private int mHeight = -1;


    protected EGLSurfaceBase(EGLHelper eglHelper) {
        this.eglHelper = eglHelper;
    }


    public void createWindowSurface(Object surface){
        if(mEGLSurface != EGL14.EGL_NO_SURFACE){
            throw new IllegalStateException("surface already created");
        }
        mEGLSurface = eglHelper.createWindowSurface(surface);
    }

    public void createOffScreenSurface(int width, int height){
        if(mEGLSurface != EGL14.EGL_NO_SURFACE){
            throw new IllegalStateException("surface already created");
        }
        mEGLSurface = eglHelper.createOffScrrenSurface(width, height);
        mWidth = width;
        mHeight = height;
    }

    public int getmWidth() {
        if(mWidth < 0){
            return eglHelper.querSurface(mEGLSurface, EGL14.EGL_WIDTH);
        }
        return mWidth;
    }

    public int getmHeight() {
        if(mHeight < 0){
            return eglHelper.querSurface(mEGLSurface, EGL14.EGL_HEIGHT);
        }
        return mHeight;
    }

    public void releaseEGLSurface(){
        eglHelper.release(mEGLSurface);
        mEGLSurface = EGL14.EGL_NO_SURFACE;
        mHeight = mWidth = -1;
    }

    /**
     * 使我们的surface和渲染上下文Context进行绑定
     */
    public void makeCurrent(){
        eglHelper.makeCurrent(mEGLSurface);
    }

    public void makeCurrentReadFrom(EGLSurfaceBase surfaceBase){
        eglHelper.makeCurrent(mEGLSurface, surfaceBase.mEGLSurface);
    }

    public boolean swapBuffers(){
        boolean reslut = eglHelper.swapBuffers(mEGLSurface);
        if(!reslut){
            JLog.w("swap buffer failed");
        }
        return reslut;
    }

    public void setPresentTime(long nsec){
        eglHelper.setPresentationTime(mEGLSurface, nsec);
    }

    /**
     * 获取当前帧缓冲
     * @return
     */
    public ByteBuffer getCurrentFrame(){
        int width = getmWidth();
        int height = getmHeight();
        ByteBuffer buf = ByteBuffer.allocateDirect(width * height * 4);
        buf.order(ByteOrder.LITTLE_ENDIAN);
        GLES30.glReadPixels(0, 0, width, height, GLES30.GL_RGBA, GLES30.GL_UNSIGNED_BYTE, buf);
        OpenGLUtil.checkGlError("glReadPixels");
        buf.rewind();

        return buf;
    }
}
