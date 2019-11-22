package com.jz.jcamera.render;

import android.graphics.SurfaceTexture;
import android.view.Surface;

import com.jz.jcamera.opengl.EGLHelper;
import com.jz.jcamera.util.JLog;

/**
 * @author jackzhous
 * @package com.jz.jcamera.render
 * @filename WindowSurface
 * date on 2019/11/22 3:42 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class WindowSurface extends EGLSurfaceBase {
    private Surface mSurface;
    private boolean mReleaseSurface;

    public WindowSurface(EGLHelper eglHelper, Surface surface, boolean mReleaseSurface) {
        super(eglHelper);
        createWindowSurface(surface);
        mSurface = surface;
        this.mReleaseSurface = mReleaseSurface;
    }

    public WindowSurface(EGLHelper eglHelper, SurfaceTexture texture) {
        super(eglHelper);
        createWindowSurface(texture);
    }

    public void release(){
        releaseEGLSurface();
        if(mSurface != null){
            if(mReleaseSurface){
                mSurface.release();
            }
            mSurface = null;
        }
    }


    public void recreate(EGLHelper helper){
        if(mSurface == null){
            JLog.error("not ye implemented for SurfaceTexture");
        }
        eglHelper = helper;
        createWindowSurface(mSurface);
    }
}
