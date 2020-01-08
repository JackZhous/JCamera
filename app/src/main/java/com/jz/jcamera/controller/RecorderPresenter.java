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
import com.jz.jcamera.recorder.FFRecordBulder;
import com.jz.jcamera.recorder.FFmpegRecorder;
import com.jz.jcamera.util.AVFormatter;
import com.jz.jcamera.util.FileUtil;
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
                                            Camera.PreviewCallback,
                                            OnRecordListener{

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
        prepareRecord();
        isRecording = audioRecorder.startRecord();
    }

    private void prepareRecord(){
        if(recorder != null){
            recorder.release();
        }

        recorder = new FFRecordBulder(FileUtil.getVideoCachePath(getContext()))
                    .setWH(CameraParam.getInstance().expectWidth, CameraParam.getInstance().expectHeight)
                    .setmRotate(CameraParam.getInstance().orientation)
                    .setmFrameRate(25)
                    .setmPixelFormat(AVFormatter.PIXEL_FORMAT_NV21)
                    .setAudioParams(audioRecorder.getSampleRate(),
                            AVFormatter.getSampleFormat(audioRecorder.getSampleRate()),
                            audioRecorder.getChannels())
                    .setListener(this)
                    .build();
        recorder.startRecord();

    }

    public void stopRecord(){
        isRecording = false;
        audioRecorder.stop();
        cameraHelper.stopPreview();
        cameraHelper.releaseCamera();
        recorder.stopRecord();
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
            recorder.recordVIdeo(data, data.length, CameraParam.getInstance().expectWidth,
                    CameraParam.getInstance().expectHeight, AVFormatter.PIXEL_FORMAT_NV21);
        }
    }

    @Override
    public void onAudioRecorderStarted() {
        JLog.i("音频录音启动");
    }

    @Override
    public void onAudioRecorderProgress(byte[] data) {
        if(isRecording){
            recorder.recordAudio(data, data.length);
        }
    }

    @Override
    public void onAudioRecorderFinish() {
        JLog.i("音频录音结束");
    }


    public void onDestroy(){
        audioRecorder.stop();
        cameraHelper.stopPreview();
        cameraHelper.releaseCamera();
    }

    @Override
    public void onRecordStart() {
        JLog.i("java onRecordStart");
    }

    @Override
    public void onRecording(float duration) {
        JLog.i("java onRecording");
    }

    @Override
    public void onRecordFinish(boolean success, float duration) {
        JLog.i("java onRecordFinish");
    }

    @Override
    public void onRecordError(String msg) {
        JLog.i("java onRecordError");
    }
}
