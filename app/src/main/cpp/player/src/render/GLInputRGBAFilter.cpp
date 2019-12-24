//
// Created by jackzhous on 2019/9/29.
//

#include "GLInputRGBAFilter.h"
#include "GLInputFilter.h"

const std::string kABGRFragmentShader = STRINGS(
        precision mediump float;
        uniform sampler2D inputTexture;
        varying vec2 textureCoordinate;

        void main() {
            vec4 abgr = texture2D(inputTexture, textureCoordinate);
            gl_FragColor = abgr;
            gl_FragColor.r = abgr.b;
            gl_FragColor.b = abgr.r;
        }
);


GLInutRGBAFilter::GLInutRGBAFilter() {
    for(int i = 0; i < 1; i++){
        inputTextureHandle[i] = 0;
        texture[i] = 0;
    }
}

GLInutRGBAFilter::~GLInutRGBAFilter() {}


void GLInutRGBAFilter::initProgram() {
    initProgram(defaultVertexShader.c_str(), kABGRFragmentShader.c_str());
}

void GLInutRGBAFilter::initProgram(const char *vertexShader, const char *fragmentShader) {
    GLFilter::initProgram(vertexShader, fragmentShader);

    if(isInitialized()){
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glUseProgram(programHandle);

        if(texture[0] == 0){
            glGenTextures(1, texture);
            for(int i = 0; i < 1; i++){
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, texture[i]);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
        }
    }
}

GLboolean GLInutRGBAFilter::uploadTexture(Texture *texture) {
    //4字节对齐
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glUseProgram(programHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture[0]);
    //对于YUV来说，数据格式是GL_LUMINANCE亮度值，而对于BGRA来说，这个则是颜色通道值
    // pixels中存放的数据是BGRABGRABGRA方式排列的，这里除4是为了求出对齐后的宽度，也就是每个颜色通道的数值
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->pitches[0] / 4 ,
                    texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE
                    , texture->pixels[0]);

    return GL_TRUE;
}

GLboolean GLInutRGBAFilter::renderTexture(Texture *texture, float *vertices,
                                          float *textureVertices) {
    if(!texture || !isInitialized()){
        return GL_FALSE;
    }

    //绑定属性
    bindAttribute(vertices, textureVertices);
    //绘制前处理
    DrawBefore();
    onDraw();
    DrawAfter();
    unbindVertexAttribute();
    unbindTexture();
    glUseProgram(0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 0);

    return GL_TRUE;

}