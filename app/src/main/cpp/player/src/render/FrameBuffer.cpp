//
// Created by jackzhous on 2019/9/9.
//

#include "FrameBuffer.h"


TextureAttributes FrameBuffer::defaultTextureAttributes = {
        .minFilter = GL_LINEAR,
        .magFilter = GL_LINEAR,
        .wrapS = GL_CLAMP_TO_EDGE,
        .wrapT = GL_CLAMP_TO_EDGE,
        .internalFormat = GL_RGBA,
        .format = GL_RGBA,
        .type = GL_UNSIGNED_BYTE
};

FrameBuffer::FrameBuffer(int width, int height, const TextureAttributes attributes) {
    this->width = width;
    this->height = height;
    this->textureAttributes = textureAttributes;
    initialized = false;
}


FrameBuffer::~FrameBuffer() {
    destroy();
}

void FrameBuffer::destroy() {
    if(texture >= 0){
        glDeleteTextures(1, &texture);
        texture = -1;
    }
    if(frameBuffer >= 0){
        glDeleteFramebuffers(1, &frameBuffer);
        frameBuffer = -1;
    }
}

void FrameBuffer::init() {
    createFrameBuffer();
}

void FrameBuffer::createFrameBuffer() {
    if(initialized){
        return;
    }

    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    createTexture();
    glTexImage2D(GL_TEXTURE_2D, 0, textureAttributes.internalFormat, width, height, 0,textureAttributes.format, textureAttributes.type, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    initialized = true;
}


void FrameBuffer::createTexture() {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureAttributes.minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureAttributes.magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureAttributes.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureAttributes.wrapT);
}

void FrameBuffer::bindBuffer() {
    if(frameBuffer >= 0){
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    }
    glViewport(0, 0, width, height);
}

void FrameBuffer::unbindBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}