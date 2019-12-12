package com.jz.jcamera.render;

import android.content.Context;
import android.util.SparseArray;

import com.jz.jcamera.camera.Camera2Manager;
import com.jz.jcamera.camera.CameraParam;
import com.jz.jcamera.util.JLog;
import com.jz.jcamera.util.OpenGLUtil;
import com.jz.jcamera.util.ScaleType;

import java.nio.FloatBuffer;

/**
 * @author jackzhous
 * @package com.jz.jcamera.opengl
 * @filename RenderManager
 * date on 2019/11/22 10:41 AM
 * @describe OPENGL 渲染 sader 以及数据配置
 * @email jackzhouyu@foxmail.com
 **/
public class RenderManager {

    private Context context;
    private FloatBuffer vertexBuffer;
    private FloatBuffer textureBuffer;
    //显示纹理裁剪的缓存顶点
    private FloatBuffer displayVertexBuffer;
    private FloatBuffer displayTextureBuffer;

    private SparseArray<BaseFilter> filterArrays = new SparseArray<>();

    // 输入图像大小
    private int mTextureWidth = 1080, mTextureHeight = 1920;
    //显示图像大小
    private int mViewWidth, mViewHeight;

    //坐标缓冲
    private ScaleType scaleType = ScaleType.CENTER_CROP;

    private RenderManager() {
    }

    public static RenderManager getInstance(){
        return Holder.instance;
    }

    private static class Holder{
        static RenderManager instance = new RenderManager();
    }

    public void releaseBuffer(){
        if(vertexBuffer != null){
            vertexBuffer.clear();
            vertexBuffer = null;
        }

        if(textureBuffer != null){
            textureBuffer.clear();
            textureBuffer = null;
        }

        if(displayVertexBuffer != null){
            displayVertexBuffer.clear();
            displayVertexBuffer = null;
        }

        if(displayTextureBuffer != null){
            displayTextureBuffer.clear();
            displayTextureBuffer = null;
        }
    }

    public void init(Context context){
        this.context = context;
        initBuffer();
        initFilters();
    }

    private void initBuffer(){
        releaseBuffer();
        vertexBuffer = OpenGLUtil.createFloatBuffer(OpenGLUtil.vertextData);
        textureBuffer = OpenGLUtil.createFloatBuffer(OpenGLUtil.fragmentData);
        displayVertexBuffer = OpenGLUtil.createFloatBuffer(OpenGLUtil.vertextData);
        displayTextureBuffer = OpenGLUtil.createFloatBuffer(OpenGLUtil.fragmentData);
    }


    private void initFilters(){
        releaseFilters();
        filterArrays.put(BaseFilter.CameraIndex, new GLImageOESInputFilter(context));
        filterArrays.put(1, new GLSplitScreenGuassFilter(context));
//        filterArrays.put(2, new BaseFilter(context));
    }

    private void releaseFilters(){
        for(int i = 0; i < filterArrays.size(); i++){
            filterArrays.get(i).release();
        }
        filterArrays.clear();
    }

    /**
     * 设置输入纹理大小
     * @param width
     * @param height
     */
    public void setTextureSize(int width, int height) {
        mTextureWidth = width;
        mTextureHeight = height;

    }

    /**
     * 调整滤镜
     */
    private void onFilterChanged(){
        for(int i = 0; i < filterArrays.size(); i++){
            filterArrays.get(i).onInputSizeChanged(mTextureWidth, mTextureHeight);
            filterArrays.get(i).onDisplaySizeChanged(mViewWidth, mViewHeight);
            filterArrays.get(i).initFrameBuffer(mTextureWidth, mTextureHeight);
            JLog.i("texture width height " + mTextureWidth + " " + mTextureHeight);
            JLog.i("view width height " + mViewWidth + " " + mViewHeight);
        }
    }

    public void setDisplaySize(int width, int height){
        mViewHeight = height;
        mViewWidth = width;
        adjustCoordinate();
        onFilterChanged();
    }

    /**
     * surface和surfaceView尺寸不一致，调整大小
     */
    private void adjustCoordinate(){
        float[] textureCoord = null;
        float[] vertextCoord = null;
        float[] textureVertices = OpenGLUtil.fragmentData;
        float[] vertexVertices = OpenGLUtil.vertextData;
        float rationMax = Math.max((float)mViewWidth / mTextureWidth
                , (float)mViewHeight/mTextureHeight);
        //新的款高 计算出来的是输出纹理宽高
        int imageWidth = Math.round(mTextureWidth * rationMax);
        int imageHeight = Math.round(mTextureHeight * rationMax);
        // 获取视图跟texture的宽高比
        float ratioWidth = (float)imageWidth / (float)mViewWidth;
        float ratioHeight = (float)imageHeight / (float)mViewHeight;
        //center inside 显示纹理小于视图宽高
        if(scaleType == ScaleType.CENTER_INSIDE){
            vertextCoord = new float[]{
                    vertexVertices[0] / ratioHeight, vertexVertices[1] / ratioWidth,
                    vertexVertices[2] / ratioHeight, vertexVertices[3] / ratioWidth,
                    vertexVertices[4] / ratioHeight, vertexVertices[5] / ratioWidth,
                    vertexVertices[6] / ratioHeight, vertexVertices[7] / ratioWidth,
            };
            //center crop以宽高比撑慢view，裁剪视图之外的部分,不过以下的算法不懂  算出来不是这个效果
        }else if (scaleType == ScaleType.CENTER_CROP){
            float distHorizontal = (1 - 1 / ratioWidth) / 2;
            float distVertical = (1 - 1 / ratioHeight) / 2;
            textureCoord = new float[] {
                    addDistance(textureVertices[0], distHorizontal), addDistance(textureVertices[1], distVertical),
                    addDistance(textureVertices[2], distHorizontal), addDistance(textureVertices[3], distVertical),
                    addDistance(textureVertices[4], distHorizontal), addDistance(textureVertices[5], distVertical),
                    addDistance(textureVertices[6], distHorizontal), addDistance(textureVertices[7], distVertical),
            };
        }

        if(vertextCoord == null){
            vertextCoord = vertexVertices;
        }

        if(textureCoord == null){
            textureCoord = textureVertices;
        }

        displayTextureBuffer.clear();
        displayTextureBuffer.put(textureCoord).position(0);
        displayVertexBuffer.clear();
        displayVertexBuffer.put(vertextCoord).position(0);
    }


    /**
     * 计算距离
     * @param coordinate
     * @param distance
     * @return
     */
    private float addDistance(float coordinate, float distance) {
        return coordinate == 0.0f ? distance : 1 - distance;
    }


    public void drawFrame(int textureId, float[] matrix){
        int currentId = textureId;
        BaseFilter filter = filterArrays.get(0);
        if(filter instanceof GLImageOESInputFilter){
            ((GLImageOESInputFilter) filter).setmTransformMatrix(matrix);
        }
        for(int i = 0; i < filterArrays.size() - 1; i++){
            //这里没有使用返回id导致画面没有画出来
            currentId = filter.drawFrameBuffer(currentId, vertexBuffer, textureBuffer);
        }

        filterArrays.get(1).drawFrame(currentId, displayVertexBuffer, displayTextureBuffer);
    }

    /**
     * 释放资源
     */
    public void release() {
        releaseBuffer();
        releaseFilters();
        context = null;
    }
}
