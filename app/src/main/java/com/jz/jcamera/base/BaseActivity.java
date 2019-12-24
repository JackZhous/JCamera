package com.jz.jcamera.base;

import android.os.Bundle;
import android.support.annotation.IdRes;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Toast;

/**
 * @author jackzhous
 * @package com.jz.jcamera.base
 * @filename BaseActivity
 * date on 2019/11/21 11:19 AM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public abstract class BaseActivity extends AppCompatActivity {

    protected abstract int provideLayout();

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(provideLayout());
        init();
    }


    protected void init(){}

    public final <T extends View> T $(@IdRes int id) {
        return this.getDelegate().findViewById(id);
    }

    protected void showMessage(String msg){
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
    }

}
