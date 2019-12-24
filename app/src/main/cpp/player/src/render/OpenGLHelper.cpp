//
// Created by jackzhous on 2019/9/9.
//

#include "OpenGLHelper.h"

GLuint OpenGLHelper::createProgram(const char *vertexShader, const char *fragmentShader) {
    GLuint vertex;
    GLuint fragment;
    GLuint program;
    GLint linked;

    //顶点着色器
    vertex = loadShader(GL_VERTEX_SHADER, vertexShader);
    if(vertex == 0){
        return 0;
    }

    //片元着色器
    fragment = loadShader(GL_FRAGMENT_SHADER, fragmentShader);
    if(fragment == 0){
        return 0;
    }

    program = glCreateProgram();
    if(program == 0){
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return 0;
    }

    //绑定shader
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if(!linked){
        GLint len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        if(len > 0){
            char* log = (char *)malloc(len * sizeof(char));
            glGetProgramInfoLog(program, len, NULL, log);
            LOGE("create program failed, the reason %s", log);
            free(log);
        }
        glDetachShader(program, vertex);
        glDeleteShader(vertex);
        glDetachShader(program, fragment);
        glDeleteShader(fragment);
        glDeleteProgram(program);
        return 0;
    }

    glDetachShader(program, vertex);
    glDeleteShader(vertex);
    glDetachShader(program, fragment);
    glDeleteShader(fragment);
    return program;

}


GLuint OpenGLHelper::loadShader(GLenum type, const char *shaderSrc) {
    GLuint shader;
    GLint compiled;

    //创建shader
    shader = glCreateShader(type);
    if(shader == 0){
        LOGE("create shader %d failed", type);
        return 0;
    }

    glShaderSource(shader, 1, &shaderSrc, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled){
        GLint  infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 0){
            char* log = (char*)malloc(infoLen * sizeof(char));
            glGetShaderInfoLog(shader, infoLen, NULL, log);
            LOGE("shader failed, the Log: %s", log);
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

void OpenGLHelper::checkActiveUniform(GLuint program) {}

GLuint OpenGLHelper::createTexture(GLenum type) {}

GLuint OpenGLHelper::createTextureWithBytes(unsigned char *bytes, int width, int height) {
    GLuint textureId;
    if(bytes == NULL){
        return 0;
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

    //设置放大缩小的模式
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return textureId;
}

GLuint OpenGLHelper::createTextureWithOldTexture(GLuint texture, unsigned char *bytes, int width,
                                                 int height) {}

void OpenGLHelper::createFrameBuffer(GLuint *framebuffer, GLuint *texture, int width, int height) {}


void OpenGLHelper::createFrameBuffers(GLuint *frameBuffers, GLuint *textures, int width, int height,
                                      int size) {
    glGenFramebuffers(size, frameBuffers);

    glGenTextures(size, textures);
    for (int i = 0; i < size; ++i) {
        glBindTexture(GL_TEXTURE_2D, textures[i]);

        // 创建一个没有像素的的Texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        // 设置放大缩小模式
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // 创建完成后需要解绑
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void OpenGLHelper::checkError(const char *op) {
    for (GLint error = glGetError(); error; error = glGetError()) {
        LOGE("[GLES2] after %s() glError (0x%x)\n", op, error);
    }
}

void OpenGLHelper::bindTexture(int location, int texture, int index) {}

void OpenGLHelper::bindTexture(int location, int texture, int index, int textureType) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(textureType, texture);

    //location 是颜色选择去 绑定到对应的纹理目标
    glUniform1f(location, index);
}

