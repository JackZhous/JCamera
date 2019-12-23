package com.jz.jcamera.util;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Environment;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class FileUtil {

    /**
     * 获取图片缓存位置
     * @param context
     * @return
     */
    public static String getImagePath(Context context){
        String path;
        if(Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState())){
            path = context.getExternalCacheDir().getAbsolutePath();
            JLog.i("external path is " + path);
        }else {
            path = context.getCacheDir().getAbsolutePath();
            JLog.i("cache dir is " + path);
        }
        path += File.separator + "JCamer_" + System.currentTimeMillis() + "jpeg";
        File file = new File(path);
        if(!file.getParentFile().exists()){
            file.getParentFile().mkdir();
        }

        return path;
    }


    public static boolean saveBitmap(String path, ByteBuffer buffer, int width, int height){
        boolean success = false;
        BufferedOutputStream instream = null;
        try {
            instream = new BufferedOutputStream(new FileOutputStream(path));
            Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            bitmap.copyPixelsFromBuffer(buffer);
            bitmap.compress(Bitmap.CompressFormat.JPEG, 100, instream);
            success = true;
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            success = false;
        }finally {
            if(instream != null){
                try {
                    instream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return success;
    }

}
