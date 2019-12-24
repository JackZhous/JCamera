//
// Created by jackzhous on 2019/9/9.
//

#ifndef MYPLAYER_FRAMEBUFFER_H
#define MYPLAYER_FRAMEBUFFER_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

typedef struct {
    GLenum minFilter;
    GLenum magFilter;
    GLenum wrapS;
    GLenum wrapT;
    GLenum format;
    GLenum internalFormat;
    GLenum type;
} TextureAttributes;

class FrameBuffer{

public:
    FrameBuffer(int width, int height, const TextureAttributes attributes = defaultTextureAttributes);
    ~FrameBuffer();

    void init();

    void destroy();

    void bindBuffer();

    void unbindBuffer();

    GLuint getTexture() const {
        return texture;
    }

    int getWidth() const {
        return width;
    }

    int getHeight() const {
        return height;
    }

    bool isInit() const {
        return initialized;
    }
    static TextureAttributes defaultTextureAttributes;

private:
    GLuint texture;             //纹理，附着到frmaebuffer上去，激活fbo后，其他texture绘制后，可以通过当前这个纹理附着获取绘制的内容
    int width, height;
    bool initialized;
    GLuint frameBuffer;
    TextureAttributes textureAttributes;

private:
    void createTexture();

    void createFrameBuffer();
};

#endif //MYPLAYER_FRAMEBUFFER_H
