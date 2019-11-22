package com.jz.jcamera.render;

import android.content.Context;
import android.util.SparseArray;

import com.jz.jcamera.util.OpenGLUtil;

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

    private SparseArray<BaseFilter> filterArrays = new SparseArray<>();

    // 输入图像大小
    private int mTextureWidth, mTextureHeight;

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
    }


    private void initFilters(){
        releaseFilters();
        filterArrays.put(BaseFilter.CameraIndex, new GLImageOESInputFilter(context));
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

}
