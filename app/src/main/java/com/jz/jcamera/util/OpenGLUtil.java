package com.jz.jcamera.util;

import android.content.Context;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLES30;
import android.util.Log;

import com.jz.jcamera.util.JLog;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

/**
 * @author jackzhous
 * @package com.jz.jcamera.opengl
 * @filename OpenGLUtil
 * date on 2019/11/22 10:17 AM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class OpenGLUtil {
    // 从初始化失败
    public static final int GL_NOT_INIT = -1;
    // 没有Texture
    public static final int GL_NOT_TEXTURE = -1;
    public static final int CoordsPerVertex = 2;
    private static final int SIZEOF_FLOAT = 4;
    private static final int SIZEOF_SHORT = 2;

    public static float[] vertextData = new float[]{
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f
    };

    public static float[] fragmentData = new float[]{
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f
    };

    /**
     * 创建Sampler2D的Framebuffer 和 Texture
     * @param frameBuffer
     * @param frameBufferTexture
     * @param width
     * @param height
     */
    public static void createFrameBuffer(int[] frameBuffer, int[] frameBufferTexture,
                                         int width, int height) {
        GLES30.glGenFramebuffers(frameBuffer.length, frameBuffer, 0);
        GLES30.glGenTextures(frameBufferTexture.length, frameBufferTexture, 0);
        for (int i = 0; i < frameBufferTexture.length; i++) {
            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, frameBufferTexture[i]);
            GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_RGBA, width, height, 0,
                    GLES30.GL_RGBA, GLES30.GL_UNSIGNED_BYTE, null);
            GLES30.glTexParameterf(GLES30.GL_TEXTURE_2D,
                    GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR);
            GLES30.glTexParameterf(GLES30.GL_TEXTURE_2D,
                    GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_LINEAR);
            GLES30.glTexParameterf(GLES30.GL_TEXTURE_2D,
                    GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_CLAMP_TO_EDGE);
            GLES30.glTexParameterf(GLES30.GL_TEXTURE_2D,
                    GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_CLAMP_TO_EDGE);
            GLES30.glBindFramebuffer(GLES30.GL_FRAMEBUFFER, frameBuffer[i]);
            GLES30.glFramebufferTexture2D(GLES30.GL_FRAMEBUFFER, GLES30.GL_COLOR_ATTACHMENT0,
                    GLES30.GL_TEXTURE_2D, frameBufferTexture[i], 0);
            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, 0);
            GLES30.glBindFramebuffer(GLES30.GL_FRAMEBUFFER, 0);
        }
        checkGlError("createFrameBuffer");
    }

    public static int createOESTexture(){
        return createTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES);
//        return createTexture(GLES30.GL_TEXTURE_2D);
    }

    private static int createTexture(int type){
        int[] textures = new int[1];
        GLES30.glGenTextures(1, textures, 0);
        checkGlError("glGenTextures");
        int textureId = textures[0];
        GLES30.glBindTexture(type, textureId);
        checkGlError("glBindTexture "+ type);
        GLES30.glTexParameterf(type, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_NEAREST);
        GLES30.glTexParameterf(type, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR);
        GLES30.glTexParameterf(type, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_CLAMP_TO_EDGE);
        GLES30.glTexParameterf(type, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_CLAMP_TO_EDGE);
        return textureId;
    }


    public static FloatBuffer createFloatBuffer(float[] data){
        FloatBuffer floatBuffer = ByteBuffer.allocateDirect(data.length * SIZEOF_FLOAT)
                                .order(ByteOrder.nativeOrder())
                                .asFloatBuffer();

       floatBuffer.put(data);
       floatBuffer.position(0);
       return floatBuffer;
    }

    /**
     * 获取shader着色器
     * @param shaderType
     * @param source
     * @return
     */
    public static int loadShader(int shaderType, String source){
        int shader = GLES30.glCreateShader(shaderType);
        checkGlError("glCreateShader type="+shaderType);
        GLES30.glShaderSource(shader, source);
        GLES30.glCompileShader(shader);
        int[] compiled = new int[1];
        GLES30.glGetShaderiv(shader, GLES30.GL_COMPILE_STATUS, compiled, 0);
        if(compiled[0] == 0){
            JLog.error("could not compiled shader, type " + shaderType);
            GLES30.glDeleteShader(shader);
            shader = 0;
        }
        return shader;
    }

    public static int createProgram( String vertexId, String fragmentId){
        int vertexShader = loadShader(GLES30.GL_VERTEX_SHADER, vertexId);
        if(vertexShader == 0){
            return 0;
        }

        int fragmentShader = loadShader(GLES30.GL_FRAGMENT_SHADER, fragmentId);
        if(fragmentShader == 0){
            return 0;
        }

        int program = GLES30.glCreateProgram();
        checkGlError("glCreateProgram");
        if(program == 0){
            JLog.error("glCreate program failed");
        }
        GLES30.glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        GLES30.glAttachShader(program, fragmentShader);
        checkGlError("glAttachShader");
        GLES30.glLinkProgram(program);
        int[] linkStatus = new int[1];
        GLES30.glGetProgramiv(program, GLES30.GL_LINK_STATUS, linkStatus, 0);
        if(linkStatus[0] != GLES30.GL_TRUE){
            JLog.error("glLinkedProgram failed");
            GLES30.glDeleteProgram(program);
            program = 0;
        }
        if(vertexShader > 0){
            GLES30.glDetachShader(program, vertexShader);
            GLES30.glDeleteShader(vertexShader);
        }
        if(fragmentShader > 0){
            GLES30.glDetachShader(program, fragmentShader);
            GLES30.glDeleteShader(fragmentShader);
        }
        return program;
    }

    public static String readRawFile(Context context, int resId){
        if(context == null){
            return null;
        }

        InputStream stream = null;
        BufferedReader reader = null;
        InputStreamReader inputStreamReader = null;
        StringBuilder sb = new StringBuilder();


        try {
            stream = context.getResources().openRawResource(resId);
            inputStreamReader = new InputStreamReader(stream);
            reader = new BufferedReader(inputStreamReader);
            String line;
            while ((line = reader.readLine()) != null){
                sb.append(line);
                sb.append("\n");
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if(stream != null){
                    stream.close();
                }
                if(inputStreamReader != null){
                    inputStreamReader.close();
                }
                if(reader != null){
                    reader.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        return sb.toString();

    }


    /**
     * 检查是否出错
     * @param op
     */
    public static void checkGlError(String op) {
        int error = GLES30.glGetError();
        if (error != GLES30.GL_NO_ERROR) {
            String msg = op + ": glError 0x" + Integer.toHexString(error);
            JLog.i(msg);
//            throw new RuntimeException(msg);
        }
    }

}
