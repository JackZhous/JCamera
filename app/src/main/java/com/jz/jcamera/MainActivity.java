package com.jz.jcamera;

import android.Manifest;
import android.graphics.SurfaceTexture;
import android.os.Build;
import android.os.Handler;
import android.support.annotation.RequiresApi;
import android.view.TextureView;
import android.view.View;
import android.widget.TextView;

import com.jz.jcamera.base.BaseActivity;
import com.jz.jcamera.camera.Camera2Manager;
import com.jz.jcamera.camera.CameraParam;
import com.jz.jcamera.controller.CamerPresenter;
import com.jz.jcamera.controller.VCallback;
import com.jz.jcamera.util.JLog;
import com.tbruyelle.rxpermissions2.Permission;
import com.tbruyelle.rxpermissions2.RxPermissions;

import io.reactivex.functions.Consumer;

public class MainActivity extends BaseActivity implements View.OnClickListener,
                                                            TextureView.SurfaceTextureListener,
                                                            VCallback {


    TextureView textureView;
    TextView fps;
    private CamerPresenter presenter;

    @Override
    protected int provideLayout() {
        return R.layout.activity_main;
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    @Override
    protected void init() {
        presenter = CamerPresenter.getInstance();
        presenter.init(this);
        presenter.setMainV(this);
        initView();
        initPermission();
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

    private void initPermission(){
        RxPermissions permissions = new RxPermissions(this);
        permissions.request(Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                    .subscribe(granted ->{
                        if(!granted){
                            showMessage("获取权限失败!");
                        }
                    });
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
