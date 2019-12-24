//
// Created by jackzhous on 2019/9/4.
//

#include "GLFilter.h"

GLFilter::GLFilter() {

}

GLFilter::~GLFilter() {}

void GLFilter::onDraw() {
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertextCount);
}

void GLFilter::setDisplaySize(int width, int height) {
    displayHeight = height;
    displayWidth = width;
}

void GLFilter::setTextureSize(int width, int height) {
    textureWidth = width;
    textureHeight = height;
}

void GLFilter::setInitialized(bool init) {
    initialized = init;
}

void GLFilter::setIntensity(float intensity) {
    this->intensity = intensity;
}

void GLFilter::setTimeStamp(double time) {
    timeStamp = time;
}

void GLFilter::initProgram() {
    if(!initialized){
        initProgram(defaultVertexShader.c_str(), defaultFragmentShader.c_str());
    }
}

void GLFilter::initProgram(const char *vertexShader, const char *fragmentShader) {
    if(initialized){
        return;
    }

    if(vertexShader && fragmentShader){
        programHandle = OpenGLHelper::createProgram(vertexShader, fragmentShader);
        vertextHanlde = glGetAttribLocation(programHandle, "aPosition");
        textureHandle = glGetAttribLocation(programHandle, "aTextureCoord");
        inputTextureHandle[0] = glGetUniformLocation(programHandle, "inputTexture");
        initialized = true;
    } else{
        programHandle = -1;
        vertextHanlde = -1;
        textureHandle = -1;
        inputTextureHandle[0] = -1;
        initialized = false;
    }
}

void GLFilter::destroyProgram() {
    if(initialized){
        glDeleteProgram(programHandle);
    }
    programHandle = -1;
}

void GLFilter::updateViewPort() {
    if(displayWidth && displayHeight){
        glViewport(0, 0, displayWidth, displayHeight);
    } else{
        glViewport(0, 0, textureWidth, textureHeight);
    }
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLFilter::bindTexture(GLuint texture) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1f(inputTextureHandle[0], 0);
}

void GLFilter::bindAttribute(const float *vertex, const float *textureVertex) {
    //设置顶点坐标
    glVertexAttribPointer(vertextHanlde, 2, GL_FLOAT, GL_FALSE, 0, vertex);
    glEnableVertexAttribArray(vertextHanlde);

    //设置纹理坐标
    glVertexAttribPointer(textureHandle, 2 ,GL_FLOAT, GL_FALSE, 0, textureVertex);
    glEnableVertexAttribArray(textureHandle);
}

void GLFilter::unbindVertexAttribute() {
    glDisableVertexAttribArray(vertextHanlde);
    glDisableVertexAttribArray(textureHandle);
}

void GLFilter::unbindTexture() {
    glBindTexture(getTextureType(), 0);
}

GLenum GLFilter::getTextureType() {
    return GL_TEXTURE_2D;
}

void GLFilter::drawTexture(GLuint texture, const float *vertex, const float *textureVertex,
                           bool update) {
    if(!initialized || texture < 0){
        return;
    }

    if(update){
        updateViewPort();
    }

    glUseProgram(programHandle);
    bindTexture(texture);
    bindAttribute(vertex, textureVertex);
    DrawBefore();
    onDraw();
    DrawAfter();
    unbindVertexAttribute();
    unbindTexture();
    glUseProgram(0);
}

void GLFilter::drawTexture(FrameBuffer *frameBuffer, GLuint texture, const float *vertex,
                           const float *textureVertex) {
    if(frameBuffer){
        frameBuffer->bindBuffer();
    }
    drawTexture(texture, vertex, textureVertex);
    if(frameBuffer){
        frameBuffer->unbindBuffer();
    }
}

void GLFilter::DrawAfter() {

}

void GLFilter::DrawBefore() {

}

bool GLFilter::isInitialized() {
    return initialized;
}