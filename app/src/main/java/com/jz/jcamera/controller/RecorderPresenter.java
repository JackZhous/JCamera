package com.jz.jcamera.controller;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.media.AudioRecord;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import com.jz.jcamera.audio.AudioRecorder;
import com.jz.jcamera.camera.CameraHelper;
import com.jz.jcamera.camera.CameraManager;
import com.jz.jcamera.camera.CameraParam;
import com.jz.jcamera.recorder.FFmpegRecorder;
import com.jz.jcamera.util.JLog;

import java.lang.ref.WeakReference;


/**
 * @author jackzhous
 * @package com.jz.jcamera.controller
 * @filename RecorderPresenter
 * date on 2019/12/24 3:26 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class RecorderPresenter implements SurfaceTexture.OnFrameAvailableListener,
                                            AudioRecorder.AudioRecorderCallback,
                                            Camera.PreviewCallback {

    FFmpegRecorder recorder;
    private CameraManager cameraHelper;
    WeakReference<RecorderCallBack> mainWeak;
    private AudioRecorder audioRecorder;
    private Handler handler;
    boolean isRecording;


    public RecorderPresenter(RecorderCallBack context) {
        mainWeak = new WeakReference<>(context);
        cameraHelper = new CameraManager();
        handler = new Handler(Looper.myLooper());
        recorder = new FFmpegRecorder(handler);
        audioRecorder = new AudioRecorder();
    }



    public void openCamera(){
        audioRecorder.setCallback(this);
        cameraHelper.openCamera(mainWeak.get().getContext(), handler);
        if(mainWeak.get() != null){
            mainWeak.get().updateTextureSize(CameraParam.getInstance().expectWidth,
                                                CameraParam.getInstance().expectHeight);
        }
    }


    public void bindSurfaceTexture(SurfaceTexture surfaceTexture){
        surfaceTexture.setOnFrameAvailableListener(this);
        cameraHelper.setPreviewCallback(surfaceTexture);
        cameraHelper.setPreviewCallback(this);

    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        if(mainWeak.get() != null){
            mainWeak.get().surfaceAvalable();
        }
    }


    public void startRecord(){
        isRecording = audioRecorder.startRecord();
    }

    public void stopRecord(){
        audioRecorder.stop();
        cameraHelper.stopPreview();
        cameraHelper.releaseCamera();
        isRecording = false;
    }


    public Context getContext(){
        if(mainWeak.get() != null){
            return mainWeak.get().getContext();
        }
        return null;
    }



    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        if(isRecording){
            JLog.i("视频data"+data.length);
        }
    }

    @Override
    public void recorderStarted() {

    }

    @Override
    public void recorderProgress(byte[] data) {
        if(isRecording){
            JLog.i("音频data"+data.length);
        }
    }

    @Override
    public void recorderFinish() {

    }


    public void onDestroy(){
        audioRecorder.stop();
        cameraHelper.stopPreview();
        cameraHelper.releaseCamera();
    }
}
