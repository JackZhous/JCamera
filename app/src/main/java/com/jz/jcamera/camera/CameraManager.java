package com.jz.jcamera.camera;

import android.app.Activity;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.os.Handler;
import android.util.Size;
import android.view.Surface;

import com.jz.jcamera.render.RenderHandler;
import com.jz.jcamera.util.JLog;
import com.jz.jcamera.util.OpenGLUtil;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * @author jackzhous
 * @package com.jz.jcamera.camera
 * @filename CameraManager
 * date on 2019/11/21 11:25 AM
 * @describe
 * 相机管理类
 * Camera.Parameters 相机配置参数，如闪光模式、预览窗口大小、对焦等；
 * 它内部是一个LinkedHashMap<String, String>结构体，存储每个特性，
 * 而get的属性时确实一个List，实质也是通过map获取属性后，在字符串split分割为list的
 * @email jackzhouyu@foxmail.com
 **/
public class CameraManager implements CameraHelper{

    private Camera camera;
    //摄像头id
    private int camerId = 1;
    // 视图宽高
    private int viewWidth;
    private int viewHeight;
    // 输入图像大小
    private int mTextureWidth, mTextureHeight;
    private Handler handler;

    public CameraManager() {
    }

    @Override
    public void setHandler(Handler handler) {
        this.handler = handler;
    }



    @Override
    public void initCamera(SurfaceTexture texture) {
    }

    @Override
    public void startPreview(Handler handler) {
        if(camera == null || texture == null){
            return;
        }
        JLog.i("startPreview");
        camera.startPreview();
    }

    @Override
    public void openCamera(Context context, Handler handler){
        try {
            //Camera.CameraInfo.CAMERA_FACING_FRONT
            camera = Camera.open(camerId);
        } catch (Exception e) {
            e.printStackTrace();
            JLog.i("open camera failed");
            return;
        }
        Camera.Parameters parameters = camera.getParameters();
        parameters.setRecordingHint(true);
        camera.setParameters(parameters);
        setPreviewSize(camera, CameraParam.getInstance().expectWidth, CameraParam.getInstance().expectHeight);
        setPictureSize(camera, CameraParam.getInstance().expectWidth, CameraParam.getInstance().expectHeight);
        camera.setDisplayOrientation(calculateCameraPreviewOrientation((Activity) context));
        handler.sendEmptyMessage(RenderHandler.MSG_CAMERA_OPENDED);
    }

    private int calculateCameraPreviewOrientation(Activity activity) {
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(camerId, info);
        int rotation = activity.getWindowManager().getDefaultDisplay()
                .getRotation();
        int degrees = 0;
        switch (rotation) {
            case Surface.ROTATION_0:
                degrees = 0;
                break;
            case Surface.ROTATION_90:
                degrees = 90;
                break;
            case Surface.ROTATION_180:
                degrees = 180;
                break;
            case Surface.ROTATION_270:
                degrees = 270;
                break;
        }

        int result;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360;
        } else {
            result = (info.orientation - degrees + 360) % 360;
        }
        CameraParam.getInstance().orientation = result;
        return result;
    }

    public void setPreviewSurface(SurfaceTexture texture){
        if(camera == null){
            JLog.i("设置相机预览surface失败");
            return;
        }
        try {
            camera.setPreviewTexture(texture);
        } catch (IOException e) {
            e.printStackTrace();
            JLog.i("设置相机预览surface失败");
        }
    }

    public void setPreviewCallback(Camera.PreviewCallback callback){
        if(camera != null){
            camera.setPreviewCallback(callback);
        }
    }
    /**
     * 调整由于surface的大小与SurfaceView大小不一致带来的显示问题
     */
    private void adjustCoordinateSize(){
        float[] textureCoord = null;
        float[] vertexCoord = null;
        float[] textureVertices = OpenGLUtil.fragmentData;
        float[] vertexVertices = OpenGLUtil.vertextData;
        float rrationMax = Math.max((float)viewWidth / mTextureWidth, (float)viewHeight / mTextureHeight);

        //新宽高
        int imagWidth = Math.round(mTextureWidth * rrationMax);
        int imagheight = Math.round(mTextureHeight * rrationMax);
    }





    public void stopPreview(){
        if(camera == null){
            return;
        }

        camera.stopPreview();
    }

    public void releaseCamera(){
        if(camera == null){
            return;
        }
        camera.release();
    }

    private SurfaceTexture texture = null;
    @Override
    public void setPreviewCallback(SurfaceTexture texture) {
        try {
            this.texture = texture;
            camera.setPreviewTexture(texture);
            startPreview(handler);
        } catch (IOException e) {
            e.printStackTrace();
            JLog.i("设置相机预览surface失败");
        }
    }

    private boolean checkSupportFalsh(Camera.Parameters parameters){
        if(parameters == null){
            return false;
        }

        List<String> list = parameters.getSupportedFlashModes();
        if(list == null || list.size() == 0 || list.size() > 0
                && list.get(0).equals(Camera.Parameters.FLASH_MODE_OFF)){
            return false;
        }
        return true;
    }


    private void setPictureSize(Camera camera, int expWidth, int expHeight){
        Camera.Parameters parameters = camera.getParameters();
        Camera.Size size = calculatePerfectSize(parameters.getSupportedPictureSizes(),
                            expWidth, expHeight, CalculateType.Max);
        parameters.setPictureSize(1280, 720);
        camera.setParameters(parameters);
    }

    private void setPreviewSize(Camera camera, int expWidth, int expHeight){
        Camera.Parameters parameters = camera.getParameters();
        Camera.Size size = calculatePerfectSize(parameters.getSupportedPreviewSizes(),
                            expWidth, expHeight, CalculateType.Lower);
        parameters.setPreviewSize(size.width, size.height);
        CameraParam.getInstance().previewWidth = size.width;
        CameraParam.getInstance().previewHeight = size.height;
        camera.setParameters(parameters);
    }

    /**
     * 计算最完美的Size
     * @param sizes
     * @param expectWidth
     * @param expectHeight
     * @return
     */
    private static Camera.Size calculatePerfectSize(List<Camera.Size> sizes, int expectWidth,
                                                    int expectHeight, CalculateType calculateType) {

        Collections.sort(sizes, new CompareAreaSize());// 根据宽度进行排序

        // 根据当前期望的宽高判定
        List<Camera.Size> bigEnough = new ArrayList<>();
        List<Camera.Size> noBigEnough = new ArrayList<>();
        for (Camera.Size size : sizes) {
            if (size.height * expectWidth / expectHeight == size.width) {
                if (size.width > expectWidth && size.height > expectHeight) {
                    bigEnough.add(size);
                } else {
                    noBigEnough.add(size);
                }
            }
        }
        // 根据计算类型判断怎么如何计算尺寸
        Camera.Size perfectSize = null;
        switch (calculateType) {
            // 直接使用最小值
            case Min:
                // 不大于期望值的分辨率列表有可能为空或者只有一个的情况，
                // Collections.min会因越界报NoSuchElementException
                if (noBigEnough.size() > 1) {
                    perfectSize = Collections.min(noBigEnough, new CompareAreaSize());
                } else if (noBigEnough.size() == 1) {
                    perfectSize = noBigEnough.get(0);
                }
                break;

            // 直接使用最大值
            case Max:
                // 如果bigEnough只有一个元素，使用Collections.max就会因越界报NoSuchElementException
                // 因此，当只有一个元素时，直接使用该元素
                if (bigEnough.size() > 1) {
                    perfectSize = Collections.max(bigEnough, new CompareAreaSize());
                } else if (bigEnough.size() == 1) {
                    perfectSize = bigEnough.get(0);
                }
                break;

            // 小一点
            case Lower:
                // 优先查找比期望尺寸小一点的，否则找大一点的，接受范围在0.8左右
                if (noBigEnough.size() > 0) {
                    Camera.Size size = Collections.max(noBigEnough, new CompareAreaSize());
                    if (((float)size.width / expectWidth) >= 0.8
                            && ((float)size.height / expectHeight) > 0.8) {
                        perfectSize = size;
                    }
                } else if (bigEnough.size() > 0) {
                    Camera.Size size = Collections.min(bigEnough, new CompareAreaSize());
                    if (((float)expectWidth / size.width) >= 0.8
                            && ((float)(expectHeight / size.height)) >= 0.8) {
                        perfectSize = size;
                    }
                }
                break;

            // 大一点
            case Larger:
                // 优先查找比期望尺寸大一点的，否则找小一点的，接受范围在0.8左右
                if (bigEnough.size() > 0) {
                    Camera.Size size = Collections.min(bigEnough, new CompareAreaSize());
                    if (((float)expectWidth / size.width) >= 0.8
                            && ((float)(expectHeight / size.height)) >= 0.8) {
                        perfectSize = size;
                    }
                } else if (noBigEnough.size() > 0) {
                    Camera.Size size = Collections.max(noBigEnough, new CompareAreaSize());
                    if (((float)size.width / expectWidth) >= 0.8
                            && ((float)size.height / expectHeight) > 0.8) {
                        perfectSize = size;
                    }
                }
                break;
        }
        // 如果经过前面的步骤没找到合适的尺寸，则计算最接近expectWidth * expectHeight的值
        if (perfectSize == null) {
            Camera.Size result = sizes.get(0);
            boolean widthOrHeight = false; // 判断存在宽或高相等的Size
            // 辗转计算宽高最接近的值
            for (Camera.Size size : sizes) {
                // 如果宽高相等，则直接返回
                if (size.width == expectWidth && size.height == expectHeight
                        && ((float) size.height / (float) size.width) == CameraParam.getInstance().currentRatio) {
                    result = size;
                    break;
                }
                // 仅仅是宽度相等，计算高度最接近的size
                if (size.width == expectWidth) {
                    widthOrHeight = true;
                    if (Math.abs(result.height - expectHeight) > Math.abs(size.height - expectHeight)
                            && ((float) size.height / (float) size.width) == CameraParam.getInstance().currentRatio) {
                        result = size;
                        break;
                    }
                }
                // 高度相等，则计算宽度最接近的Size
                else if (size.height == expectHeight) {
                    widthOrHeight = true;
                    if (Math.abs(result.width - expectWidth) > Math.abs(size.width - expectWidth)
                            && ((float) size.height / (float) size.width) == CameraParam.getInstance().currentRatio) {
                        result = size;
                        break;
                    }
                }
                // 如果之前的查找不存在宽或高相等的情况，则计算宽度和高度都最接近的期望值的Size
                else if (!widthOrHeight) {
                    if (Math.abs(result.width - expectWidth) > Math.abs(size.width - expectWidth)
                            && Math.abs(result.height - expectHeight) > Math.abs(size.height - expectHeight)
                            && ((float) size.height / (float) size.width) == CameraParam.getInstance().currentRatio) {
                        result = size;
                    }
                }
            }
            perfectSize = result;
        }
        return perfectSize;
    }

    /**
     * 比较器
     */
    private static class CompareAreaSize implements Comparator<Camera.Size> {
        @Override
        public int compare(Camera.Size pre, Camera.Size after) {
            return Long.signum((long) pre.width * pre.height -
                    (long) after.width * after.height);
        }
    }
}
