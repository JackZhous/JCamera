package com.jz.jcamera.render;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.opengl.GLES30;
import android.os.Build;
import android.os.HandlerThread;
import android.util.Size;
import android.view.Surface;

import com.jz.jcamera.camera.Camera2Manager;
import com.jz.jcamera.camera.CameraHelper;
import com.jz.jcamera.camera.CameraManager;
import com.jz.jcamera.camera.CameraParam;
import com.jz.jcamera.camera.FpsHelper;
import com.jz.jcamera.controller.PCallBack;
import com.jz.jcamera.opengl.EGLHelper;
import com.jz.jcamera.util.JLog;
import com.jz.jcamera.util.OpenGLUtil;

import java.nio.ByteBuffer;

/**
 * @author jackzhous
 * @package com.jz.jcamera.render
 * @filename RenderThread
 * date on 2019/11/22 2:36 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class RenderThread extends HandlerThread implements SurfaceTexture.OnFrameAvailableListener, Camera.PreviewCallback {
    private int mInputTexture;
    private int mCurrentTexture;
    //surfaceTextures可以接受来自相机预览和视频解码的视频流
    private SurfaceTexture surfaceTexture;
    private CameraHelper cameraHelper;
    //矩阵
    private final float[] mMatrix = new float[16];

    private Context context;

    private RenderManager renderManager;
    private EGLHelper eglHelper;
    // 预览用的EGLSurface
    private WindowSurface mDisplaySurface;

    private final Object mSyncOperation = new Object();
    private boolean isPreviewing = false;
    private boolean isRecording = false;
    private CameraParam cameraParam = CameraParam.getInstance();

    private RenderHandler renderHandler;
    private FpsHelper fpsHelper;
    private PCallBack pCallBack;

    private final Object lockCamera = new Object();

    // 输入图像大小
    private int mTextureWidth, mTextureHeight;

    public RenderThread(String name, Context context) {
        super(name);
        this.context = context;
        renderManager = RenderManager.getInstance();
        if(Build.VERSION.SDK_INT < 21){
            cameraHelper = new CameraManager();
        }else {
            cameraHelper = new Camera2Manager(context);
        }
        fpsHelper = new FpsHelper();
    }

    public void setpCallBack(PCallBack pCallBack) {
        this.pCallBack = pCallBack;
    }

    public void setRenderHandler(RenderHandler renderHandler) {
        this.renderHandler = renderHandler;
        cameraHelper.setHandler(renderHandler);
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
//        openCamera();
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        synchronized (mSyncOperation){
            //刷新数据
            if(isPreviewing || isRecording){
                renderHandler.sendMessage(renderHandler.obtainMessage(RenderHandler.MSG_PREVIEW_CALLBACK, data));
            }


        }
    }
    /**
     * 预览回调
     * @param data
     */
    void onPreviewCallback(byte[] data) {
        if (cameraParam.cameraCallback != null) {
            cameraParam.cameraCallback.onPreviewCallback(data);
        }

        drawFrame();
    }


    void drawFrame(){
        //计算fps
        caluteFps();
        if(mDisplaySurface == null || surfaceTexture == null){
            return;
        }

        mDisplaySurface.makeCurrent();
        //surface关联的opengl的纹理对象将被更新为最新的图像，图像来自相机预览或者视频解码，必须在opengl渲染线程中调用
        surfaceTexture.updateTexImage();
        //纹理转换矩阵
        surfaceTexture.getTransformMatrix(mMatrix);
        renderManager.drawFrame(mInputTexture, mMatrix);

        mDisplaySurface.swapBuffers();
    }

    public void surfaceDestroyed(){
        mDisplaySurface.makeCurrent();
        renderManager.release();
        releaseCamera();
        if (surfaceTexture != null) {
            surfaceTexture.release();
            surfaceTexture = null;
        }
        if (mDisplaySurface != null) {
            mDisplaySurface.release();
            mDisplaySurface = null;
        }
        if (eglHelper != null) {
            eglHelper.release();
            eglHelper = null;
        }
    }



    void surfaceCreate(Surface surface){
        eglHelper = new EGLHelper();
        mDisplaySurface = new WindowSurface(eglHelper, surface, false);
        mDisplaySurface.makeCurrent();

        GLES30.glDisable(GLES30.GL_DEPTH_TEST); //深度测试，绘制像素时如果有别的像素挡住他 就不绘制
        GLES30.glDisable(GLES30.GL_CULL_FACE);  //不是正面的都会被剔除掉

        renderManager.init(context);
        mInputTexture = OpenGLUtil.createOESTexture();      //创建OES类型的纹理，目前只知道能自动yuv->rgb格式转换
        surfaceTexture = new SurfaceTexture(mInputTexture);
        surfaceTexture.setOnFrameAvailableListener(this);
    }

    void surfaceCreate(SurfaceTexture surface){
        eglHelper = new EGLHelper();
        mDisplaySurface = new WindowSurface(eglHelper, surface);
        mDisplaySurface.makeCurrent();

        GLES30.glDisable(GLES30.GL_DEPTH_TEST); //深度测试，绘制像素时如果有别的像素挡住他 就不绘制
        GLES30.glDisable(GLES30.GL_CULL_FACE);  //不是正面的都会被剔除掉

        renderManager.init(context);
        mInputTexture = OpenGLUtil.createOESTexture();      //创建OES类型的纹理，目前只知道能自动yuv->rgb格式转换
        surfaceTexture = new SurfaceTexture(mInputTexture);
        surfaceTexture.setOnFrameAvailableListener(this);
        openCamera();
    }


    private void openCamera(){
        releaseCamera();
        cameraHelper.initCamera(surfaceTexture);
        cameraHelper.openCamera(context, renderHandler);
    }

    void setPreview(){
        cameraHelper.setPreviewCallback(surfaceTexture);
        if(cameraHelper instanceof  CameraManager){
            ((CameraManager) cameraHelper).setPreviewCallback(this);
        }
        calculateImageSize();
        if(cameraParam.cameraCallback != null){
            cameraParam.cameraCallback.onCameraOpened();
        }
    }

    private void calculateImageSize(){
        if(cameraParam.orientation == 90 || cameraParam.orientation == 270){
            mTextureWidth = cameraParam.previewHeight;
            mTextureHeight = cameraParam.previewWidth;
        }else {
            mTextureWidth = cameraParam.previewWidth;
            mTextureHeight = cameraParam.previewHeight;
        }
        renderManager.setTextureSize(mTextureWidth, mTextureHeight);
    }

    private void releaseCamera(){
        isPreviewing = false;
        cameraHelper.stopPreview();
        cameraHelper.releaseCamera();
    }

    void surfaceChanged(int width, int height){
        renderManager.setDisplaySize(width, height);
        startPreview();
    }

    private void startPreview(){
        cameraHelper.startPreview(renderHandler);
        isPreviewing = true;
    }


    protected void setEffectId(int id){
        renderManager.setEffectId(id);
    }

    /**
     * 计算fps
     */
    protected void caluteFps(){
        fpsHelper.caluteFps();
        if(pCallBack != null){
            pCallBack.showFps(fpsHelper.getFps());
        }
    }

    protected void takePhoto(){
        synchronized (lockCamera){
            ByteBuffer buffer = mDisplaySurface.getCurrentFrame();

            if(pCallBack != null){
                pCallBack.takePhotoSuccess(buffer, mDisplaySurface.getmWidth(), mDisplaySurface.getmHeight());
            }
        }
    }
}
