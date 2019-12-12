package com.jz.jcamera.camera;


import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.net.wifi.aware.Characteristics;
import android.os.Build;
import android.os.Handler;
import android.support.annotation.RequiresApi;
import android.util.Size;
import android.view.Surface;

import com.jz.jcamera.render.RenderHandler;
import com.jz.jcamera.util.JLog;

import java.util.Arrays;
import java.util.Map;

import static com.jz.jcamera.camera.CameraParam.DEFAULT_16_9_HEIGHT;
import static com.jz.jcamera.camera.CameraParam.DEFAULT_16_9_WIDTH;


@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
public class Camera2Manager implements CameraHelper{

    private Context context;
    private CameraManager service;
    private CameraParam paramC;
    private CameraDevice device;
    private CameraCaptureSession session;
    private CaptureRequest.Builder builder;
    private Surface surface;
    private Handler handler;
    private boolean previewTask;
    private String cameraId = "1";

    public Camera2Manager(Context context){
        this.context = context;
        paramC = CameraParam.getInstance();
        service = (CameraManager) context.getSystemService(Context.CAMERA_SERVICE);
    }

    public void setHandler(Handler handler) {
        this.handler = handler;
    }

    public void initCamera(SurfaceTexture texture){
        try {
            //0是后置摄像头
            CameraCharacteristics param = service.getCameraCharacteristics(cameraId);
            StreamConfigurationMap map = param.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            Size[] size = map.getOutputSizes(SurfaceTexture.class);     //获取预览尺寸类型
//            for(Size s : size){
//                if(s.getHeight() == paramC.expectHeight && s.getWidth() == paramC.expectWidth){
//                    texture.setDefaultBufferSize(s.getWidth(), s.getHeight());
//                    paramC.previewHeight = s.getHeight();
//                    paramC.previewWidth = s.getWidth();
//                    return;
//                }
//            }
//            paramC.previewHeight = size[0].getHeight();
//            paramC.previewWidth = size[0].getWidth();
            JLog.i("width height camera " + size[0].getWidth()+ "  "+size[0].getHeight());
//            texture.setDefaultBufferSize(paramC.expectHeight, paramC.expectWidth);
            texture.setDefaultBufferSize(size[0].getWidth(), size[0].getHeight());
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }


    @SuppressLint("MissingPermission")
    @Override
    public void openCamera(Context context, Handler handler) {
        try {
            service.openCamera(cameraId, stateCallback, handler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

    }


    private CameraDevice.StateCallback stateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened( CameraDevice camera) {
            device = camera;
            handler.sendEmptyMessage(RenderHandler.MSG_CAMERA_OPENDED);
        }

        @Override
        public void onDisconnected( CameraDevice camera) {

        }

        @Override
        public void onError( CameraDevice camera, int error) {

        }
    };


    @Override
    public void setPreviewCallback(SurfaceTexture texture) {
        try {
            builder = device.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        surface = new Surface(texture);
        builder.addTarget(surface);
        if(previewTask){
            startPreview(handler);
        }
    }

    @Override
    public void startPreview( Handler handler) {
        if(device == null){
            previewTask = true;
            return;
        }
        try {
            device.createCaptureSession(Arrays.asList(surface), new CameraCaptureSession.StateCallback() {
                @Override
                public void onConfigured(CameraCaptureSession session) {
                    try {
                        Camera2Manager.this.session = session;
                        session.setRepeatingRequest(builder.build(), new CameraCaptureSession.CaptureCallback() {
                            @Override
                            public void onCaptureStarted( CameraCaptureSession session, CaptureRequest request, long timestamp, long frameNumber) {
                                super.onCaptureStarted(session, request, timestamp, frameNumber);
                            }

                            @Override
                            public void onCaptureProgressed( CameraCaptureSession session, CaptureRequest request, CaptureResult partialResult) {
                                super.onCaptureProgressed(session, request, partialResult);
                            }

                            @Override
                            public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request, TotalCaptureResult result) {
                                super.onCaptureCompleted(session, request, result);
                                JLog.i("onCaptureCompleted");
                                handler.sendMessage(handler.obtainMessage(RenderHandler.MSG_DRAW_FRAME));
                            }
                        }, handler);
                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }
                }

                @Override
                public void onConfigureFailed( CameraCaptureSession session) {

                }
            }, handler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void stopPreview() {
        try {
            if(session != null){
                session.abortCaptures();
            }

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void releaseCamera() {
        if(session != null){
            session.close();
        }
        if(device != null){
            device.close();
        }
    }
}
