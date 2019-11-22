package com.jz.jcamera.util;

import android.util.Log;

/**
 * @author jackzhous
 * @package com.jz.jcamera.util
 * @filename JLog
 * date on 2019/11/21 3:25 PM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class JLog {

    private static final String TAG = "j_tag";

    public static void i(String msg){
        Log.i(TAG, msg);
    }

    public static void w(String msg){
        Log.w(TAG, msg);
    }


    public static void error(String msg){
        Log.i(TAG, msg);
        throw new RuntimeException(msg);
    }
}
