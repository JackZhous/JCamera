package com.jz.jcamera.controller;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.Looper;
import android.widget.TextView;

import com.jz.jcamera.camera.CameraParam;
import com.jz.jcamera.render.RenderHandler;
import com.jz.jcamera.render.RenderThread;
import com.jz.jcamera.util.FileUtil;
import com.jz.jcamera.util.JLog;
import com.jz.jcamera.util.ToastHelper;

import java.nio.ByteBuffer;

/**
 * @author jackzhous
 * @package com.jz.jcamera.controller
 * @filename CamerPresenter
 * date on 2019/11/22 5:07 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public final class CamerPresenter implements PCallBack {

    private CameraParam cameraParam;

    private RenderHandler renderHandler;
    private RenderThread renderThread;
    private Handler mainHandler;
    private VCallback mainV;
    private Context context;


    private static class RenderHolder {
        private static CamerPresenter instance = new CamerPresenter();
    }

    public void setMainV(VCallback mainV) {
        this.mainV = mainV;
    }

    private CamerPresenter() {
        cameraParam = CameraParam.getInstance();
    }



    public static CamerPresenter getInstance(){
        return RenderHolder.instance;
    }

    public void init(Context context){
        this.context = context;
        renderThread = new RenderThread("renderThread", context);
        renderThread.setpCallBack(this);
        renderThread.start();
        renderHandler = new RenderHandler(renderThread);
        renderThread.setRenderHandler(renderHandler);
        mainHandler = new Handler(Looper.getMainLooper());
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

    @Override
    public void showFps(float fps) {
        if(mainV == null){
            return;
        }
        mainHandler.post(() -> {
            if(mainV.getFpsView() != null){
                JLog.i("show fps " + fps);
                mainV.getFpsView().setText(String.valueOf(fps));
            }
        });
    }


    public void onRelease(){
        mainV = null;
        mainHandler = null;
    }

    public void takePhoto(){
        renderHandler.sendEmptyMessage(RenderHandler.MSG_TAKE_PICTURE);
    }

    @Override
    public void takePhotoSuccess(ByteBuffer buffer, int width, int height) {
        String path = FileUtil.getImagePath(context);
        boolean flag = FileUtil.saveBitmap(path, buffer, width, height);
        if(flag){
            ToastHelper.showMsg(context, "图片保存在："+path);
        }
    }
}
