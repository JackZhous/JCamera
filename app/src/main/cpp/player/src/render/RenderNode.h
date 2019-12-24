//
// Created by jackzhous on 2019-09-11.
//

#ifndef MYPLAYER_RENDERNODE_H
#define MYPLAYER_RENDERNODE_H

#include "FrameBuffer.h"
#include "GLFilter.h"
#include "RenderNodeType.h"

/**
 * 渲染结点
 * 数据流如下：
 *   input->render node 1->render node 2->render node 3->...->render node n->output
 * texture->frameBuffer 1->frameBuffer 2->frameBuffer 3->...->frameBuffer n->texture
 *        ->glFilter     ->glFilter     ->glFilter     ->...->glFilter     ->glFilter
 *
 * 每个渲染结点上的glFilter均可以随时改变，而FrameBuffer则不需要跟随glFilter一起销毁重建，节省开销
 */
class RenderNode{
public:
    RenderNode(RenderNodeType renderType);
    ~RenderNode();

    void init();

    void destroy();
    //设置纹理大小
    void setTextureSize(int width, int height);
    //设置显示大小
    void setDisplaySize(int width, int height);

    void setFrameBuffer(FrameBuffer* buffer);

    //切换Filter
    void changeFilter(GLFilter *filter);
    //设置时间戳
    void setTimeStamp(double timeStamp);

    //设置强度
    void setIntensity(float intensity);

    //直接绘制输出
    virtual bool drawFrame(GLuint texture, const float *vertices, const float *textureVertices);

    virtual int drawFrameBuffer(GLuint texture, const float *vertices, const float *textureVertices);

    RenderNodeType getNodeType() const ;

    bool hasFrameBuffer() const ;

public:
    RenderNode* prevNode;       //前端节点

    RenderNode* nextNode;       //下一个渲染节点

    RenderNodeType nodeType;

protected:
    int textureWidth, textureHeight;        //纹理宽高

    int displayWidth, displayHeight;        //显示宽高
    //滤镜
    GLFilter* glFilter;
    //FrameBuffer对象
    FrameBuffer* frameBuffer;
};

#endif //MYPLAYER_RENDERNODE_H
