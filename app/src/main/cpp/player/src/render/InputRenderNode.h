//
// Created by jackzhous on 2019-09-11.
//

#ifndef MYPLAYER_INPUTRENDERNODE_H
#define MYPLAYER_INPUTRENDERNODE_H

#include "GLInputFilter.h"
#include "RenderNode.h"
#include "CoordinateUtils.h"
#include "GLInputYUV420Filter.h"

/**
 * 内含GLFilter对象，操作此对象进行纹理数据上传和绘制
 * 没看出此类有什么具体的作用，估计是接口编程，根据类型确定不同的GLFilter，如YUV420或者RGBA等fliter
 * 在进行绘制
 */

class InputRenderNode : public RenderNode{
    public:
        InputRenderNode();
        ~InputRenderNode();

        void initFilter(Texture* texture);

        //上传纹理
        bool uploadTexture(Texture *texture);

        //绘制纹理
        bool drawFrame(Texture* texture);

        //将纹理绘制到FBO
        int drawFrameBuffer(Texture* texture);

    private:
        bool drawFrame(GLuint texture, const float *vertices, const float *textureVertices) override ;

        int drawFrameBuffer(GLuint texture, const float *vertices, const float *textureVertices) override ;

        void resetVertices();

        void resetTextureVertices(Texture* texture);

        RotationMode getRotateMode(Texture* texture);

        //裁剪纹理数据，帧宽度和纹理宽度不一致时
        void cropTexVertices(Texture* texture);

    private:
        GLfloat vertices[8];
        GLfloat textureVertice[8];
};

#endif //MYPLAYER_INPUTRENDERNODE_H
