package com.jz.jcamera.controller;

import android.content.Context;

/**
 * @author jackzhous
 * @package com.jz.jcamera.controller
 * @filename RecorderCallBack
 * date on 2019/12/25 10:05 AM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public interface RecorderCallBack {

    Context getContext();


    void surfaceAvalable();


    void updateTextureSize(int width, int height);
}
