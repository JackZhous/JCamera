package com.jz.jcamera.camera;

/**
 * @author jackzhous
 * @package com.jz.jcamera.camera
 * @filename OnCameraCallback
 * date on 2019/11/22 5:01 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public interface OnCameraCallback {
    // 相机已打开
    void onCameraOpened();

    // 预览回调
    void onPreviewCallback(byte[] data);
}
