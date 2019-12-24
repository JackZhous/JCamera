//
// Created by jackzhous on 2019/9/10.
//

#include "GLInputFilter.h"

GLInputFilter::GLInputFilter() {}

GLInputFilter::~GLInputFilter() {}

GLboolean GLInputFilter::uploadTexture(Texture *texture) {
    return GL_TRUE;
}

GLboolean GLInputFilter::renderTexture(Texture *texture, float *vertices, float *textureVertices) {
    return GL_TRUE;
}



void GLInputFilter::drawTexture(GLuint texture, const float *vertex, const float *textureVertex,
                                bool update) {
}

void GLInputFilter::drawTexture(FrameBuffer *frameBuffer, GLuint texture, const float *vertex,
                                const float *textureVertex) {
    GLFilter::drawTexture(frameBuffer, texture, vertex, textureVertex);
}
