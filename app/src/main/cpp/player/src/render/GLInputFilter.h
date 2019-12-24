//
// Created by jackzhous on 2019/9/10.
//

#ifndef MYPLAYER_GLINPUTFILTER_H
#define MYPLAYER_GLINPUTFILTER_H

#include "GLFilter.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>

/**
 * 纹理图片格式
 */
typedef enum {
    FMT_NONE = -1,
    FMT_YUV420P,
    FMT_ARGB
} TextureFormat;


/*
 * 翻转模式
 */
typedef enum {
    FLIP_NONE = 0x00,
    FLIP_HORIZONTAL = 0x01,
    FLIP_VERTICAL = 0x02
} FlipDirection;

/**
 * 设置混合模式
 */
typedef enum {
    BLEND_NONE = 0x00,
    BLEND_NORMAL = 0x01,
    BLEND_ADD = 0x02,
    BLEND_MODULATE = 0x03
} BlendMode;

#define NUM_DATA_POINTERS 3
#define GLES_MAX_PLANE 3
/**
 * 纹理结构体
 */
typedef struct {
    int width;
    int height;
    int frameWidth;
    int frameHeight;
    int rotate;                     //渲染角度
    BlendMode blendMode;            // 混合模式，主要是方便后续添加字幕渲染之类的。字幕是绘制到图像上的，需要开启混合模式。
    FlipDirection direction;
    TextureFormat format;
    uint16_t pitches[NUM_DATA_POINTERS];            //宽对齐
    uint8_t *pixels[NUM_DATA_POINTERS];             //像素数据
} Texture;

class GLInputFilter : public GLFilter{
    public:
        GLInputFilter();
        virtual ~GLInputFilter();

        virtual GLboolean uploadTexture(Texture* texture);

        virtual GLboolean renderTexture(Texture* texture, float *vertices, float* textureVertices);

protected:
    //绘制纹理
    virtual void drawTexture(GLuint texture, const float* vertex, const float* textureVertex, bool update = false) override ;

    virtual void drawTexture(FrameBuffer* frameBuffer, GLuint texture, const float* vertex, const float* textureVertex) override ;

protected:
    GLuint texture[GLES_MAX_PLANE];     //纹理id
};


#endif //MYPLAYER_GLINPUTFILTER_H
