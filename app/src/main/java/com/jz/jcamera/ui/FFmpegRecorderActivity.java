package com.jz.jcamera.ui;

import android.content.Context;
import android.media.AudioManager;
import android.opengl.GLSurfaceView;
import android.view.View;

import com.jz.jcamera.R;
import com.jz.jcamera.base.BaseActivity;
import com.jz.jcamera.controller.RecorderCallBack;
import com.jz.jcamera.controller.RecorderPresenter;
import com.jz.jcamera.render.RecorderRender;
import com.jz.jcamera.util.JLog;

/**
 * @author jackzhous
 * @package com.jz.jcamera.ui
 * @filename FFmpegRecorderActivity
 * date on 2019/12/24 5:05 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class FFmpegRecorderActivity extends BaseActivity implements View.OnClickListener,
                                                                    RecorderCallBack {

    RecorderPresenter presenter;
    RecorderRender render;
    GLSurfaceView glSurface;
    private AudioManager audioManager;

    @Override
    protected int provideLayout() {
        return R.layout.activity_ffm_recorder;
    }


    @Override
    protected void init() {
        presenter = new RecorderPresenter(this);
        render = new RecorderRender(presenter);
        glSurface = $(R.id.GLSurfaceView);
        glSurface.setEGLContextClientVersion(3);
        glSurface.setRenderer(render);
        glSurface.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        findViewById(R.id.record).setOnClickListener(this);
        if (audioManager == null) {
            audioManager = (AudioManager) getContext().getSystemService(Context.AUDIO_SERVICE);
            audioManager.requestAudioFocus(null, AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN);
        }

        presenter.openCamera();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.record:

                break;
        }
    }


    @Override
    public Context getContext() {
        return this;
    }

    @Override
    public void surfaceAvalable() {
        glSurface.requestRender();
    }

    @Override
    public void updateTextureSize(int width, int height) {
        render.setTextureSize(width, height);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        presenter.onDestroy();
    }
}
