package com.jz.jcamera.util;

import android.media.AudioFormat;

/**
 * @author jackzhous
 * @package com.jz.jcamera.util
 * @filename AVFormatter
 * date on 2020/1/8 10:02 AM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public final class AVFormatter {
    // 图像像素格式
    public static final int PIXEL_FORMAT_NONE = 0;
    public static final int PIXEL_FORMAT_NV21 = 1;
    public static final int PIXEL_FORMAT_YV12 = 2;
    public static final int PIXEL_FORMAT_NV12 = 3;
    public static final int PIXEL_FORMAT_YUV420P = 4;
    public static final int PIXEL_FORMAT_ARGB = 5;
    public static final int PIXEL_FORMAT_ABGR = 6;
    public static final int PIXEL_FORMAT_RGBA = 7;

    // 音频采样格式
    public static final int SAMPLE_FORMAT_NONE = 0;
    public static final int SAMPLE_FORMAT_8BIT = 8;
    public static final int SAMPLE_FORMAT_16BIT = 16;
    public static final int SAMPLE_FORMAT_FLOAT = 32;

    /**
     * 获取采样格式
     * @param audioFormat AudioFormat格式
     * @return
     */
    public static int getSampleFormat(int audioFormat) {
        int sampleFormat = SAMPLE_FORMAT_NONE;
        switch (audioFormat) {
            case AudioFormat.ENCODING_PCM_8BIT: {
                sampleFormat = SAMPLE_FORMAT_8BIT;
                break;
            }

            case AudioFormat.ENCODING_PCM_16BIT: {
                sampleFormat = SAMPLE_FORMAT_16BIT;
                break;
            }

            case AudioFormat.ENCODING_PCM_FLOAT: {
                sampleFormat = SAMPLE_FORMAT_FLOAT;
                break;
            }
        }
        return sampleFormat;
    }

}
