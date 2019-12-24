//
// Created by jackzhous on 2019/9/10.
//

#include "GLInputYUV420Filter.h"
#include "GLInputFilter.h"

GLInputYUV420Filter::GLInputYUV420Filter() {
    for(int i = 0; i < GLES_MAX_PLANE; i++){
        inputTextureHandle[i] = 0;
        texture[i] = 0;
    }
}

GLInputYUV420Filter::~GLInputYUV420Filter() {}

void GLInputYUV420Filter::initProgram() {
    initProgram(defaultVertexShader.c_str(), yuv420FragmentShader.c_str());
}

void GLInputYUV420Filter::initProgram(const char *vertexShader, const char *fragmentShader) {
    if(vertexShader && fragmentShader){
        programHandle = OpenGLHelper::createProgram(vertexShader, fragmentShader);
        OpenGLHelper::checkError("createProgram");
        vertextHanlde = glGetAttribLocation(programHandle, "aPosition");
        textureHandle = glGetAttribLocation(programHandle, "aTextureCoord");        //这个是纹理坐标绑定
        inputTextureHandle[0] = glGetUniformLocation(programHandle, "inputTextureY");
        inputTextureHandle[1] = glGetUniformLocation(programHandle, "inputTextureU");
        inputTextureHandle[2] = glGetUniformLocation(programHandle, "inputTextureV");

        // 需要设置4字节对齐
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glUseProgram(programHandle);

        if(texture[0] == 0){
            glGenTextures(GLES_MAX_PLANE, texture);
        }
        for(int i = 0; i < GLES_MAX_PLANE; i++){
            glActiveTexture(GL_TEXTURE0 + i);   //激活纹理目标
            glBindTexture(GL_TEXTURE_2D, texture[i]);           //绑定纹理单元

            //设置过滤模式
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            //纹理颜色选择器绑定纹理，最后选择器根据纹理坐标到相应的纹理中选取颜色
            glUniform1i(inputTextureHandle[i], i);
        }
        setInitialized(true);
    } else{
        programHandle = -1;
        vertextHanlde = -1;
        inputTextureHandle[0] = -1;
        setInitialized(false);
    }
}

GLboolean GLInputYUV420Filter::uploadTexture(Texture *texture) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glUseProgram(programHandle);

    //更新绑定纹理数据
    const GLsizei heights[3] = {texture->height, texture->height / 2, texture->height / 2};
//    LOGI("glPixel ylen %d ulen %d ydatalen %d", texture->pitches[0], texture->pitches[1], sizeof(texture->pixels[0]));
    for (int i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);   //激活纹理目标
        glBindTexture(GL_TEXTURE_2D, this->texture[i]);           //绑定纹理单元
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_LUMINANCE,
                     texture->pitches[i],
                     heights[i],
                     0,
                     GL_LUMINANCE,
                     GL_UNSIGNED_BYTE,
                     texture->pixels[i]);
        glUniform1f(inputTextureHandle[i], i);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
    return GL_TRUE;
}


GLboolean GLInputYUV420Filter::renderTexture(Texture *texture, float *vertices,
                                             float *textureVertices) {
    if(!isInitialized() || !texture){
        return GL_FALSE;
    }
    //为何不用  glUseProgram(programHandle);
    //解答，因为调用这个函数之前会先调用uploadTexture，这里掉了

    bindAttribute(vertices, textureVertices);

    DrawBefore();

    onDraw();

    DrawAfter();

    unbindVertexAttribute();

    unbindTexture();

    glUseProgram(0);

    return GL_TRUE;
}
