package com.jz.jcamera.opengl;

import android.graphics.SurfaceTexture;
import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLExt;
import android.opengl.EGLSurface;
import android.util.Log;
import android.view.Surface;

import com.jz.jcamera.render.EGLSurfaceBase;
import com.jz.jcamera.util.JLog;

import javax.microedition.khronos.egl.EGL;

/**
 * @author jackzhous
 * @package com.jz.jcamera.opengl
 * @filename EGLHelper
 * date on 2019/11/21 3:53 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class EGLHelper {

    private EGLDisplay eglDisplay = EGL14.EGL_NO_DISPLAY;
    private EGLContext eglContext = EGL14.EGL_NO_CONTEXT;
    private EGLConfig eglConfig = null;
    private int eglVersion;

    /**
     * Constructor flag: surface must be recordable.  This discourages EGL from using a
     * pixel format that cannot be converted efficiently to something usable by the video
     * encoder.
     */
    public static final int FLAG_RECORDABLE = 0x01;
    // Android-specific extension.
    private static final int EGL_RECORDABLE_ANDROID = 0x3142;

    public EGLHelper() {
        init();
    }


    private void init(){
        eglDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
        if(eglDisplay == EGL14.EGL_NO_DISPLAY){
            JLog.error("eglDisplay get null");
            return;
        }

        int[] version = new int[2];
        if(!EGL14.eglInitialize(eglDisplay, version, 0, version ,1)){
            JLog.error("eglDisplay init failed");
            return;
        }

        initGL3(3, null);
        if(eglContext == EGL14.EGL_NO_CONTEXT){
            initGL2(2, null);
        }

        int[] valurs = new int[1];
        EGL14.eglQueryContext(eglDisplay, eglContext, EGL14.EGL_CONTEXT_CLIENT_VERSION, valurs, 0);
        JLog.i("EGL Context created, the version is " + valurs[0]);
    }

    /**
     * 初始话GL3版本
     * @return
     */
    private void initGL3(int flag, EGLContext sharedContext){
        EGLConfig config = getConfig(flag, 3);
        if(config != null){
            int[] attrib = {
                    EGL14.EGL_CONTEXT_CLIENT_VERSION, 3,
                    EGL14.EGL_NONE
            };
            EGLContext context = EGL14.eglCreateContext(eglDisplay, config, sharedContext, attrib, 0);
            if(EGL14.eglGetError() == EGL14.EGL_SUCCESS){
                eglContext = context;
                eglConfig = config;
                eglVersion = 3;
            }
        }
    }

    /**
     * 初始话GL3版本
     * @return
     */
    private void initGL2(int flag, EGLContext sharedContext){
        EGLConfig config = getConfig(flag, 2);
        if(config != null){
            int[] attrib = {
                    EGL14.EGL_CONTEXT_CLIENT_VERSION, 2,
                    EGL14.EGL_NONE
            };
            EGLContext context = EGL14.eglCreateContext(eglDisplay, config, sharedContext, attrib, 0);
            if(EGL14.eglGetError() == EGL14.EGL_SUCCESS){
                eglContext = context;
                eglConfig = config;
                eglVersion = 2;
            }
        }
    }


    private EGLConfig getConfig(int flag, int version){
        int renderType = EGL14.EGL_OPENGL_ES2_BIT;
        if(flag >= 3){
            renderType |= EGLExt.EGL_OPENGL_ES3_BIT_KHR;
        }
        //EGL_RENDERABLE_TYPE指定使用不同的渲染API，如GLES2、GLES3
        int[] attriList = {
                EGL14.EGL_RED_SIZE, 8,
                EGL14.EGL_GREEN_SIZE, 8,
                EGL14.EGL_BLUE_SIZE, 8,
                EGL14.EGL_ALPHA_SIZE, 8,
                EGL14.EGL_RENDERABLE_TYPE, renderType,
                EGL14.EGL_NONE, 0,
                EGL14.EGL_NONE
        };
        //支持视频录制配置
        if((flag & FLAG_RECORDABLE) != 0) {
            attriList[attriList.length - 3] = EGL_RECORDABLE_ANDROID;
            attriList[attriList.length - 2] = 1;
        }

        EGLConfig[] config = new EGLConfig[1];
        int[] numConfig = new int[1];
        if(!EGL14.eglChooseConfig(eglDisplay, attriList, 0, config, 0, config.length,
                numConfig, 0)) {
            JLog.i("unable to find RGB8888 / " + version + " EGLConfig");
            return null;
        }
        return config[0];
    }

    public void release(){
        if(eglDisplay != EGL14.EGL_NO_DISPLAY){
            EGL14.eglMakeCurrent(eglDisplay,EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_SURFACE,
                    EGL14.EGL_NO_CONTEXT);
            EGL14.eglDestroyContext(eglDisplay, eglContext);
            EGL14.eglReleaseThread();
            EGL14.eglTerminate(eglDisplay);
        }
        eglDisplay = EGL14.EGL_NO_DISPLAY;
        eglContext = EGL14.EGL_NO_CONTEXT;
        eglConfig = null;
    }


    public void release(EGLSurface surface){
        EGL14.eglDestroySurface(eglDisplay, surface);
    }

    public EGLSurface createWindowSurface(Object surface){
        if(!(surface instanceof Surface) && !(surface instanceof SurfaceTexture)){
            JLog.error("invalid surface");
        }

        int[] attribute = {EGL14.EGL_NONE};
        EGLSurface eglSurface = EGL14.eglCreateWindowSurface(eglDisplay, eglConfig, surface, attribute, 0);
        checkEglError("eglCreateWindowSurface");
        if(eglSurface == null){
            JLog.error("create surface failed");
        }
        return eglSurface;

    }

    public EGLSurface createOffScrrenSurface(int width, int height){
        int[] attribute = {
                EGL14.EGL_WIDTH, width,
                EGL14.EGL_HEIGHT, height,
                EGL14.EGL_NONE
        };

        EGLSurface surface = EGL14.eglCreatePbufferSurface(eglDisplay, eglConfig, attribute, 0);
        checkEglError("eglCreatePbufferSurface");
        if(surface == null){
            JLog.error("surface create failed");
        }

        return surface;
    }

    public int querSurface(EGLSurface surface, int what){
        int[] value = new int[1];
        EGL14.eglQuerySurface(eglDisplay, surface, what, value, 0);
        return value[0];
    }

    private void checkEglError(String msg){
        int error;
        if((error = EGL14.eglGetError()) != EGL14.EGL_SUCCESS){
            JLog.error(msg + ": EGL error: 0x" + Integer.toHexString(error));
        }
    }

    public void makeCurrent(EGLSurface surface){
        /**
         * 将surface和context渲染上下文进行绑定，surface作为openGL渲染的最终目的地
         * display则是在前端显示
         */
        EGL14.eglMakeCurrent(eglDisplay, surface, surface, eglContext);
    }


    public void makeCurrent(EGLSurface draw, EGLSurface read){
        /**
         * 将surface和context渲染上下文进行绑定，surface作为openGL渲染的最终目的地
         * display则是在前端显示
         */
        EGL14.eglMakeCurrent(eglDisplay, draw, read, eglContext);
    }

    public boolean swapBuffers(EGLSurface surface){
        return EGL14.eglSwapBuffers(eglDisplay, surface);
    }

    /**
     * Sends the presentation time stamp to EGL.  Time is expressed in nanoseconds.
     */
    public void setPresentationTime(EGLSurface eglSurface, long nsecs) {
        EGLExt.eglPresentationTimeANDROID(eglDisplay, eglSurface, nsecs);
    }
}
