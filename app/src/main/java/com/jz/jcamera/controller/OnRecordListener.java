package com.jz.jcamera.controller;

/**
 * @author jackzhous
 * @package com.jz.jcamera.controller
 * @filename OnRecordListener
 * date on 2020/1/6 11:32 AM
 * @describe 录音编码回调
 * @email jackzhouyu@foxmail.com
 **/
public interface OnRecordListener {

    void onRecordStart();

    void onRecording(float duration);

    void onRecordFinish(boolean success, float duration);

    void onRecordError(String msg);
}
