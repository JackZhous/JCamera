//
// Created by jackzhous on 2019/9/9.
//

#ifndef MYPLAYER_OPENGLHELPER_H
#define MYPLAYER_OPENGLHELPER_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include <AndroidLog.h>
#include <stdlib.h>

class OpenGLHelper{
public:
    static GLuint  createProgram(const char* vertex, const char* fragment);

    static GLuint loadShader(GLenum type, const char* shaderSrc);

    //查询活动的同一变量uniform
    static void checkActiveUniform(GLuint program);

    //创建texture
    static GLuint  createTexture(GLenum type);

    static GLuint createTextureWithBytes(unsigned char* bytes, int width, int height);

    static GLuint createTextureWithOldTexture(GLuint texture, unsigned char* bytes, int width, int height);

    //创建一个framebuffer和texture
    static void createFrameBuffer(GLuint *framebuffer, GLuint* texture, int width, int height);

    static void createFrameBuffers(GLuint* frameBuffers, GLuint* texture, int width, int height, int size);

    static void checkError(const char* op);

    static void bindTexture(int location, int texture, int index);

    static void bindTexture(int location, int texture, int index, int textureType);

private:
    //C++11的新特性，显示标明生成默认的构造函数
    OpenGLHelper() = default;
    virtual ~OpenGLHelper(){}
};
#endif //MYPLAYER_OPENGLHELPER_H