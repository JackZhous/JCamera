//
// Created by jackzhous on 2019-09-11.
//

#include "InputRenderNode.h"
#include "GLInputFilter.h"
#include "GLInputRGBAFilter.h"

InputRenderNode::InputRenderNode() : RenderNode(NODE_INPUT) {
    resetVertices();
    resetTextureVertices(NULL);
}

InputRenderNode::~InputRenderNode() {}

void InputRenderNode::initFilter(Texture* texture) {
    if(!glFilter){
        if(texture){
            if(texture->format == FMT_YUV420P){
                glFilter = new GLInputYUV420Filter();
                LOGI("create yuv filter");
            } else if(texture->format == FMT_ARGB){
                glFilter = new GLInutRGBAFilter();
                LOGI("create rgb filter");
            }
            init();
        }
    }
    if(texture){
        setTextureSize(texture->width, texture->height);
    }
}


bool InputRenderNode::uploadTexture(Texture *texture) {
    if(glFilter && glFilter->isInitialized()){
        //将视频的数据绑定到纹理上去
        return ((GLInputFilter*)glFilter)->uploadTexture(texture);
    }

    return true;
}

bool InputRenderNode::drawFrame(Texture *texture) {
    cropTexVertices(texture);
    if(glFilter != nullptr){
        return ((GLInputFilter*) glFilter)->renderTexture(texture, vertices, textureVertice);
    }
    return false;
}


int InputRenderNode::drawFrameBuffer(Texture *texture) {
    if(!frameBuffer || !frameBuffer->isInit() || !glFilter || !glFilter->isInitialized()){
        return -1;
    }

    frameBuffer->bindBuffer();
    cropTexVertices(texture);
    ((GLInputFilter*) glFilter)->renderTexture(texture, vertices, textureVertice);
    frameBuffer->unbindBuffer();

    //获取fbo上的附着纹理
    return frameBuffer->getTexture();
}

int InputRenderNode::drawFrameBuffer(GLuint texture, const float *vertices,
                                     const float *textureVertices) {
    return RenderNode::drawFrameBuffer(texture, vertices, textureVertices);
}


bool InputRenderNode::drawFrame(GLuint texture, const float *vertices,
                                const float *textureVertices) {
    return RenderNode::drawFrame(texture, vertices, textureVertices);
}

void InputRenderNode::cropTexVertices(Texture *texture) {
    if(texture && texture->width != texture->frameWidth){
        GLsizei padding = texture->width - texture->frameWidth;
        GLfloat normalized = (padding + 0.5f) / texture->width;
        const float *vertices = CoordinateUtils::getInputTextureCoordinates(getRotateMode(texture));
        textureVertice[0] = vertices[0];
        textureVertice[1] = vertices[1];
        textureVertice[2] = vertices[2] - normalized;
        textureVertice[3] = vertices[3];
        textureVertice[4] = vertices[4];
        textureVertice[5] = vertices[5];
        textureVertice[6] = vertices[6] - normalized;
        textureVertice[7] = vertices[7];
    } else{
        resetTextureVertices(texture);
    }
}

void InputRenderNode::resetVertices() {
    const float * defaultVertices = CoordinateUtils::getVertexCoordinates();
    for(int i = 0; i < 8; i++){
        vertices[i] = defaultVertices[i];
    }
}


void InputRenderNode::resetTextureVertices(Texture *texture) {
    const float *Tvertices = CoordinateUtils::getInputTextureCoordinates(getRotateMode(texture));
    for(int i = 0; i < 8; i++){
        textureVertice[i] = Tvertices[i];
    }
}

RotationMode InputRenderNode::getRotateMode(Texture *texture) {
    if(texture == nullptr){
        return RORATE_NONE;
    }
    RotationMode mode = RORATE_NONE;
    if(texture->rotate == 90){
        mode = RORATE_90;
    }else if(texture->rotate == 180){
        mode = RORATE_180;
    } else if(texture->rotate == 270){
        mode = RORATE_270;
    }

    return mode;
}

