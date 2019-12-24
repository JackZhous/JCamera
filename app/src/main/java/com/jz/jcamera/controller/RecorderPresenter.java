package com.jz.jcamera.controller;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;

import com.jz.jcamera.camera.CameraHelper;
import com.jz.jcamera.camera.CameraManager;
import com.jz.jcamera.recorder.FFmpegRecorder;


/**
 * @author jackzhous
 * @package com.jz.jcamera.controller
 * @filename RecorderPresenter
 * date on 2019/12/24 3:26 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class RecorderPresenter {

    FFmpegRecorder recorder;
    Handler handler;
    private CameraHelper cameraHelper;
    Context context;

    public RecorderPresenter(Context context) {
        this.context = context;
        handler = new Handler(Looper.myLooper());
        cameraHelper = new CameraManager();
        recorder = new FFmpegRecorder(handler);
        init();
    }


    private void init(){
        cameraHelper.openCamera(context, handler);
    }
}
