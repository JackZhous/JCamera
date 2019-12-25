package com.jz.jcamera.render;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;

import com.jz.jcamera.controller.RecorderPresenter;
import com.jz.jcamera.render.filter.BaseFilter;
import com.jz.jcamera.render.filter.GLImageOESInputFilter;
import com.jz.jcamera.ui.OpengGLTestActivity;
import com.jz.jcamera.util.JLog;
import com.jz.jcamera.util.OpenGLUtil;

import java.lang.ref.WeakReference;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * @author jackzhous
 * @package com.jz.jcamera.render
 * @filename RecorderRender
 * date on 2019/12/24 5:20 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class RecorderRender implements GLSurfaceView.Renderer {

    private WeakReference<RecorderPresenter> weakPresenter;
    private int inputTexture;
    private SurfaceTexture surfaceTexture;

    private FloatBuffer vertextBuffer;
    private FloatBuffer textureBuffer;
    private FloatBuffer displayVertexBuffer;
    private FloatBuffer displayTextureBuffer;

    private GLImageOESInputFilter cameraInputFilter;
    private BaseFilter baseFilter;
    //矩阵
    private final float[] mMatrix = new float[16];

    private int textureWidth, textureHeight;
    private int viewWidth, viewHeight;

    public RecorderRender(RecorderPresenter presenter) {
        weakPresenter = new WeakReference<>(presenter);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        vertextBuffer = OpenGLUtil.createFloatBuffer(OpenGLUtil.vertextData);
        textureBuffer = OpenGLUtil.createFloatBuffer(OpenGLUtil.fragmentData);
        displayVertexBuffer = OpenGLUtil.createFloatBuffer(OpenGLUtil.vertextData);
        displayTextureBuffer = OpenGLUtil.createFloatBuffer(OpenGLUtil.fragmentData);

        inputTexture = OpenGLUtil.createOESTexture();
        surfaceTexture = new SurfaceTexture(inputTexture);
        //禁用抖动算法，RGB565显示RGB888像素的算法，GLSurface默认是565
        gl.glDisable(GL10.GL_DITHER);
        gl.glClearColor(0, 0, 0, 0);
        //GL_DEPTH_TEST深度测试，即像素点覆盖背后点仍然要绘制
        gl.glEnable(GL10.GL_DEPTH_TEST);
        //多边形背面不被剔除
        gl.glEnable(GL10.GL_CULL_FACE);
        if(weakPresenter.get() != null){
            weakPresenter.get().bindSurfaceTexture(surfaceTexture);
        }
        initFilter();
    }


    private void initFilter(){
        cameraInputFilter = new GLImageOESInputFilter(weakPresenter.get().getContext());
        baseFilter = new BaseFilter(weakPresenter.get().getContext());
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        viewWidth = width;
        viewHeight = height;
        onFilterChanged();
        adjustCoordinateSize();
    }

    public void setTextureSize(int width, int height){
        JLog.i("setTextureSize");
        textureHeight = height;
        textureWidth = width;
    }

    private void onFilterChanged(){
        JLog.i("onFilterChanged");
        cameraInputFilter.onInputSizeChanged(textureWidth, textureHeight);
        //设置filter fbo渲染的宽高
        cameraInputFilter.initFrameBuffer(textureWidth, textureHeight);
        //设置输出窗口宽高
        cameraInputFilter.onDisplaySizeChanged(viewWidth, viewHeight);

        //设置filter fbo渲染的宽高
        baseFilter.initFrameBuffer(textureWidth, textureHeight);
        //设置输出窗口宽高
        baseFilter.onDisplaySizeChanged(viewWidth, viewHeight);
    }

    private void adjustCoordinateSize(){
        float[] textureCoord;
        float[] vertexCorrd = OpenGLUtil.vertextData;
        float[] textureVertices = OpenGLUtil.fragmentData;
        float ratioMax = Math.max((float)viewWidth/ textureWidth, (float)viewHeight/textureHeight);

        //新的宽高
        float imageWidth = textureWidth * ratioMax;
        float imageHeight = textureHeight * ratioMax;

        //获取视图宽高
        float ratioWidth = imageWidth / viewWidth;
        float ratioHeight = imageHeight / viewHeight;
        float distHorizontal = (1 - 1/ratioWidth) / 2;
        float disVertical = (1 - 1 / ratioHeight) / 2;

        textureCoord = new float[]{
                addDistance(textureVertices[0], distHorizontal), addDistance(textureVertices[1], disVertical),
                addDistance(textureVertices[2], distHorizontal), addDistance(textureVertices[3], disVertical),
                addDistance(textureVertices[4], distHorizontal), addDistance(textureVertices[5], disVertical),
                addDistance(textureVertices[6], distHorizontal), addDistance(textureVertices[7], disVertical),
        };

        displayVertexBuffer.clear();
        displayVertexBuffer.put(vertexCorrd).position(0);
        displayTextureBuffer.clear();
        displayTextureBuffer.put(textureCoord).position(0);
    }


    @Override
    public void onDrawFrame(GL10 gl) {
        if(surfaceTexture != null){
            surfaceTexture.updateTexImage();
            surfaceTexture.getTransformMatrix(mMatrix);
        }

        GLES30.glClearColor(0,0, 0, 0);
        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT | GLES30.GL_DEPTH_BUFFER_BIT);
        if (cameraInputFilter == null || baseFilter == null) {
            return;
        }

        cameraInputFilter.setmTransformMatrix(mMatrix);
        int texture = cameraInputFilter.drawFrameBuffer(inputTexture, vertextBuffer, textureBuffer);
        baseFilter.drawFrame(texture, displayVertexBuffer, displayTextureBuffer);
    }

    private float addDistance(float coordinate, float distance) {
        return coordinate == 0.0f ? distance : 1 - distance;
    }
}
