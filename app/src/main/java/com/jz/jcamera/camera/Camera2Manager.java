package com.jz.jcamera.camera;


import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.net.wifi.aware.Characteristics;
import android.os.Build;
import android.os.Handler;
import android.support.annotation.RequiresApi;
import android.util.Size;

import java.util.Map;


@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
public class Camera2Manager implements CameraHelper{

    private Context context;
    private CameraManager service;
    private CameraParam paramC;
    private CameraDevice device;

    public Camera2Manager(Context context){
        this.context = context;
        paramC = CameraParam.getInstance();
        service = (CameraManager) context.getSystemService(Context.CAMERA_SERVICE);
        initCamera();
    }


    public Size initCamera(){
        try {
            //0是后置摄像头
            CameraCharacteristics param = service.getCameraCharacteristics("0");
            StreamConfigurationMap map = param.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            Size[] size = map.getOutputSizes(SurfaceTexture.class);     //获取预览尺寸类型
            for(Size s : size){
                if(s.getHeight() == paramC.expectHeight && s.getWidth() == paramC.expectWidth){
                    return s;
                }
            }
            return size[0];
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        return null;
    }


    @SuppressLint("MissingPermission")
    @Override
    public void openCamera(Context context, Handler handler) {
        try {
            service.openCamera("0", stateCallback, handler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

    }


    private CameraDevice.StateCallback stateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened( CameraDevice camera) {
            device = camera;
        }

        @Override
        public void onDisconnected( CameraDevice camera) {

        }

        @Override
        public void onError( CameraDevice camera, int error) {

        }
    };
}
