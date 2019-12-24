package com.jz.jcamera.ui;

import android.opengl.GLSurfaceView;
import android.view.View;

import com.jz.jcamera.R;
import com.jz.jcamera.base.BaseActivity;
import com.jz.jcamera.controller.RecorderPresenter;
import com.jz.jcamera.render.RecorderRender;

/**
 * @author jackzhous
 * @package com.jz.jcamera.ui
 * @filename FFmpegRecorderActivity
 * date on 2019/12/24 5:05 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class FFmpegRecorderActivity extends BaseActivity implements View.OnClickListener {

    RecorderPresenter presenter;
    RecorderRender recorder;
    GLSurfaceView glSurface;


    @Override
    protected int provideLayout() {
        return R.layout.activity_ffm_recorder;
    }


    @Override
    protected void init() {
        presenter = new RecorderPresenter();
        recorder = new RecorderRender();
        glSurface = $(R.id.GLSurfaceView);
        findViewById(R.id.record).setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.record:

                break;
        }
    }
}
