package com.jz.jcamera.util;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Matrix;
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
        path += File.separator + "JCamer_" + System.currentTimeMillis() + ".jpeg";
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
            bitmap = rorateDegree(bitmap, 180);
            bitmap = xFlip(bitmap);
            bitmap.compress(Bitmap.CompressFormat.JPEG, 100, instream);
            success = true;
            if(bitmap.isRecycled()){
                bitmap.recycle();
                bitmap = null;
            }
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


    public static Bitmap rorateDegree(Bitmap bitmap, int degree){
        Matrix matrix = new Matrix();
        matrix.postRotate(degree);
        bitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);

        return bitmap;
    }

    /**
     * 水平翻转
     * @param bitmap
     * @return
     */
    public static Bitmap xFlip(Bitmap bitmap){
        Matrix matrix = new Matrix();
        matrix.setScale(-1, 1);
        bitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);

        return bitmap;
    }

    /**
     * 获取视频缓存绝对路径
     * @param context
     * @return
     */
    public static String getVideoCachePath(Context context) {
        String directoryPath;

        // 判断外部存储是否可用，如果不可用则使用内部存储路径
        if (Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState())) {
            directoryPath = context.getExternalCacheDir().getAbsolutePath();
        } else { // 使用内部存储缓存目录
            directoryPath = context.getCacheDir().getAbsolutePath();
        }
        String path = directoryPath + File.separator + "JCamera_" + System.currentTimeMillis() + ".mp4";
        File file = new File(path);
        if (!file.getParentFile().exists()) {
            file.getParentFile().mkdirs();
        }
        return path;
    }

}
