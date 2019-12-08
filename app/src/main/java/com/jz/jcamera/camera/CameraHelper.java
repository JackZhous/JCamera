package com.jz.jcamera.camera;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.util.Size;


public interface CameraHelper {
    //openCamera必须要和设置预览在一起，
    void openCamera(Context context, Handler handler);

    void initCamera(SurfaceTexture texture);

    void setPreviewCallback(SurfaceTexture texture);

    void startPreview(Handler handler);

    void stopPreview();

    void releaseCamera();
    void setHandler(Handler handler);
}
