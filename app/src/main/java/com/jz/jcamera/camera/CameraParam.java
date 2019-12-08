package com.jz.jcamera.camera;

import android.hardware.Camera;

/**
 * @author jackzhous
 * @package com.jz.jcamera.camera
 * @filename CameraParam
 * date on 2019/11/22 4:37 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class CameraParam {
    // 最大权重
    public static final int MAX_FOCUS_WEIGHT = 1000;
    // 录制时长(毫秒)
    public static final int DEFAULT_RECORD_TIME = 15000;

    // 16:9的默认宽高(理想值) 2048 1536 1920 1440 2160 1080
    public static final int DEFAULT_16_9_WIDTH = 1536;
    public static final int DEFAULT_16_9_HEIGHT = 2048;
    // 4:3的默认宽高(理想值)
    public static final int DEFAULT_4_3_WIDTH = 1024;
    public static final int DEFAULT_4_3_HEIGHT = 768;
    // 期望fps
    public static final int DESIRED_PREVIEW_FPS = 30;
    // 这里反过来是因为相机的分辨率跟屏幕的分辨率宽高刚好反过来
    public static final float Ratio_4_3 = 0.75f;
    public static final float Ratio_16_9 = 0.5625f;

    public int cameraId;
    // 实际预览宽度
    public int previewWidth;
    // 实际预览高度
    public int previewHeight;
    // 期望预览宽度
    public int expectWidth;
    // 期望预览高度
    public int expectHeight;
    // 当前长宽比
    public float currentRatio;
    // 预览回调
    public OnCameraCallback cameraCallback;
    private static final CameraParam mInstance = new CameraParam();
    // 预览角度
    public int orientation;

    private CameraParam() {
        reset();
    }

    /**
     * 重置为初始状态
     */
    private void reset() {
//        drawFacePoints = false;
//        showFps = false;
//        aspectRatio = AspectRatio.Ratio_16_9;
        currentRatio = Ratio_16_9;
//        expectFps = DESIRED_PREVIEW_FPS;
//        previewFps = 0;
        expectWidth = DEFAULT_16_9_WIDTH;
        expectHeight = DEFAULT_16_9_HEIGHT;
        previewWidth = 0;
        previewHeight = 0;
//        highDefinition = false;
        orientation = 0;
//        backCamera = false;
        cameraId = Camera.CameraInfo.CAMERA_FACING_FRONT;
//        supportFlash = false;
//        focusWeight = 1000;
//        recordable = true;
//        recordTime = DEFAULT_RECORD_TIME;
//        audioPermitted = false;
//        recordAudio = true;
//        touchTake = false;
//        takeDelay = false;
//        luminousEnhancement = false;
//        brightness = -1;
//        mGalleryType = GalleryType.PICTURE;
//        gallerySelectedListener = null;
//        captureListener = null;
//        musicSelectListener = null;
        cameraCallback = null;
//        captureCallback = null;
//        fpsCallback = null;
//        showCompare = false;
//        isTakePicture = false;
//        enableDepthBlur = false;
//        enableVignette = false;
//        beauty = new BeautyParam();
    }
    /**
     * 获取相机配置参数
     * @return
     */
    public static CameraParam getInstance() {
        return mInstance;
    }


}
