package com.jz.jcamera.ui;

import android.Manifest;
import android.content.Intent;
import android.graphics.SurfaceTexture;
import android.os.Build;
import android.os.Handler;
import android.support.annotation.RequiresApi;
import android.support.v7.app.AppCompatActivity;
import android.view.TextureView;
import android.view.View;
import android.widget.TextView;

import com.jz.jcamera.R;
import com.jz.jcamera.base.BaseActivity;
import com.jz.jcamera.camera.Camera2Manager;
import com.jz.jcamera.camera.CameraParam;
import com.jz.jcamera.controller.CamerPresenter;
import com.jz.jcamera.controller.VCallback;
import com.jz.jcamera.util.JLog;
import com.tbruyelle.rxpermissions2.Permission;
import com.tbruyelle.rxpermissions2.RxPermissions;

import io.reactivex.functions.Consumer;

public class MainActivity extends BaseActivity implements View.OnClickListener{

    @Override
    protected int provideLayout() {
        return R.layout.activity_main;
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
    protected void init() {
        initPermission();
        findViewById(R.id.btn_take_picture).setOnClickListener(this);
        findViewById(R.id.btn_recorder).setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.btn_take_picture:
                startActivity(new Intent(MainActivity.this, OpengGLTestActivity.class));
                break;


            case R.id.btn_recorder:
                startActivity(new Intent(MainActivity.this, FFmpegRecorderActivity.class));
                break;

        }
    }
}
