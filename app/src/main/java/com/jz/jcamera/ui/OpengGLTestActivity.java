package com.jz.jcamera.ui;

import android.graphics.SurfaceTexture;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.view.TextureView;
import android.view.View;
import android.widget.TextView;

import com.jz.jcamera.R;
import com.jz.jcamera.base.BaseActivity;
import com.jz.jcamera.camera.CameraParam;
import com.jz.jcamera.controller.CamerPresenter;
import com.jz.jcamera.controller.VCallback;
import com.jz.jcamera.util.JLog;

/**
 * @author jackzhous
 * @package com.jz.jcamera.ui
 * @filename OpengGLTestActivity
 * date on 2019/12/24 4:56 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class OpengGLTestActivity extends BaseActivity implements View.OnClickListener,
                                                                TextureView.SurfaceTextureListener,
                                                                VCallback {
    TextureView textureView;
    TextView fps;
    private CamerPresenter presenter;

    @Override
    protected int provideLayout() {
        return R.layout.activity_opengl;
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    @Override
    protected void init() {
        presenter = CamerPresenter.getInstance();
        presenter.init(this);
        presenter.setMainV(this);
        initView();
    }

    private void initView(){
        textureView = $(R.id.texture_view);
        fps = $(R.id.fps);
        $(R.id.ic_take).setOnClickListener(this);
        $(R.id.ic_tx).setOnClickListener(this);
        $(R.id.ic_lib).setOnClickListener(this);
        textureView.setSurfaceTextureListener(this);
    }


    @Override
    public TextView getFpsView() {
        return fps;
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            //拍照
            case R.id.ic_take:
                presenter.takePhoto();
                break;

            //特效
            case R.id.ic_tx:

                break;

            //图库
            case R.id.ic_lib:

                break;
        }
    }




    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        JLog.i("texture view onSurfaceTextureAvailable");
        CameraParam.getInstance().previewWidth = width;
        CameraParam.getInstance().previewHeight = height;
        presenter.bindSurface(surface);
        presenter.changePreviewSize(width, height);
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        presenter.changePreviewSize(width, height);
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        presenter.unBindSurface();
        return false;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {

    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        presenter.onRelease();
    }
}
