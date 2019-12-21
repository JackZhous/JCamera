package com.jz.jcamera.render;

import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.Message;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.lang.ref.WeakReference;

/**
 * @author jackzhous
 * @package com.jz.jcamera.render
 * @filename RenderHandler
 * date on 2019/11/22 2:47 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class RenderHandler extends Handler {
    // Surface创建
    public static final int MSG_SURFACE_CREATED = 0x001;
    // Surface改变
    public static final int MSG_SURFACE_CHANGED = 0x002;
    // Surface销毁
    public static final int MSG_SURFACE_DESTROYED = 0x003;
    // 渲染
    public static final int MSG_RENDER = 0x004;
    // 开始录制
    public static final int MSG_START_RECORDING = 0x006;
    // 停止录制
    public static final int MSG_STOP_RECORDING = 0x008;
    // 重新打开相机
    public static final int MSG_REOPEN_CAMERA = 0x009;
    // 切换相机
    public static final int MSG_SWITCH_CAMERA = 0x010;
    // 预览帧回调
    public static final int MSG_PREVIEW_CALLBACK = 0x011;
    // 拍照
    public static final int MSG_TAKE_PICTURE = 0x012;
    // 计算fps
    public static final int MSG_CALCULATE_FPS = 0x013;
    // 切换边框模糊功能
    public static final int MSG_CHANGE_EDGE_BLUR = 0x14;
    // 切换动态滤镜
    public static final int MSG_CHANGE_DYNAMIC_COLOR = 0x15;
    // 切换动态彩妆
    public static final int MSG_CHANGE_DYNAMIC_MAKEUP = 0x16;
    // 切换动态动态资源
    public static final int MSG_CHANGE_DYNAMIC_RESOURCE = 0x17;
    //直接drawFrame
    public static final int MSG_DRAW_FRAME = 0x18;

    //相机打开
    public static final int MSG_CAMERA_OPENDED = 0x19;

    private WeakReference<RenderThread> mWeakReference;

    public RenderHandler(RenderThread renderThread) {
        super(renderThread.getLooper());
        this.mWeakReference = new WeakReference<>(renderThread);
    }

    @Override
    public void handleMessage(Message msg) {
        if(mWeakReference == null || mWeakReference.get() == null){
            return;
        }

        RenderThread thread = mWeakReference.get();
        switch (msg.what){
            case MSG_SURFACE_CREATED:
//                if (msg.obj instanceof SurfaceHolder) {
//                    thread.surfaceCreated(((SurfaceHolder)msg.obj).getSurface());
//                } else
                    if (msg.obj instanceof Surface) {
                    thread.surfaceCreate((Surface)msg.obj);
                } else if (msg.obj instanceof SurfaceTexture) {
                    thread.surfaceCreate((SurfaceTexture) msg.obj);
                }
                break;


            case MSG_SURFACE_CHANGED:
                thread.surfaceChanged(msg.arg1, msg.arg2);
                break;

            case MSG_PREVIEW_CALLBACK:
                thread.onPreviewCallback((byte[]) msg.obj);
                break;


            case MSG_SURFACE_DESTROYED:
                thread.surfaceDestroyed();
                break;


            case MSG_DRAW_FRAME:
                thread.drawFrame();
                break;

            case MSG_CAMERA_OPENDED:
                thread.setPreview();
                break;

            case MSG_CALCULATE_FPS:
                thread.caluteFps();
                break;
        }
    }
}
