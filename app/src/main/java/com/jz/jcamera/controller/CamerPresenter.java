package com.jz.jcamera.controller;

import android.content.Context;
import android.graphics.SurfaceTexture;

import com.jz.jcamera.camera.CameraManager;
import com.jz.jcamera.camera.CameraParam;
import com.jz.jcamera.render.RenderHandler;
import com.jz.jcamera.render.RenderThread;

/**
 * @author jackzhous
 * @package com.jz.jcamera.controller
 * @filename CamerPresenter
 * date on 2019/11/22 5:07 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public final class CamerPresenter {

    private CameraParam cameraParam;

    private RenderHandler renderHandler;
    private RenderThread renderThread;


    private static class RenderHolder {
        private static CamerPresenter instance = new CamerPresenter();
    }


    private CamerPresenter() {
        cameraParam = CameraParam.getInstance();
    }



    public static CamerPresenter getInstance(){
        return RenderHolder.instance;
    }

    public void init(Context context){
        renderThread = new RenderThread("renderThread", context);
        renderThread.start();
        renderHandler = new RenderHandler(renderThread);
        renderThread.setRenderHandler(renderHandler);
    }

    public void bindSurface(SurfaceTexture surfaceTexture){
        renderHandler.sendMessage(renderHandler.obtainMessage(RenderHandler.MSG_SURFACE_CREATED, surfaceTexture));
    }

    public void changePreviewSize(int width, int height){
        renderHandler.sendMessage(renderHandler.obtainMessage(RenderHandler.MSG_SURFACE_CHANGED, width, height));
    }
    /**
     * 解绑Surface
     */
    public void unBindSurface() {
        renderHandler.sendMessage(renderHandler
                .obtainMessage(RenderHandler.MSG_SURFACE_DESTROYED));
    }
}
