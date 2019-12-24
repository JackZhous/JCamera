package com.jz.jcamera.recorder;

import android.os.Handler;

import com.jz.jcamera.camera.CameraHelper;
import com.jz.jcamera.camera.CameraManager;


/**
 * @author jackzhous
 * @package com.jz.jcamera.recorder
 * @filename FFmpegRecorder
 * date on 2019/12/24 3:27 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class FFmpegRecorder {

    static {
        System.loadLibrary("ffmpeg");
        System.loadLibrary("player");
        System.loadLibrary("recorder");
    }

    //native录像管理器引擎
    private long nativeHandler;

    public native long nativeInit();



    private Handler handler;

    public FFmpegRecorder(Handler handler) {
        nativeHandler = nativeInit();
        this.handler = handler;

    }



}
