//
// Created by jackzhous on 2019/9/4.
//

#ifndef MYPLAYER_GLFILTER_H
#define MYPLAYER_GLFILTER_H
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include "common/Config.h"
#include "FrameBuffer.h"
#include <string>
#include "OpenGLHelper.h"


// OpenGLES 2.0 最大支持32个纹理
#define MAX_TEXTURES 32
//默认的顶点着色器程序
const std::string defaultVertexShader = STRINGS(
        precision mediump float;
        attribute highp vec4 aPosition;
        attribute highp vec2 aTextureCoord;
        varying vec2 textureCoordinate;
        void main() {
            gl_Position  = aPosition;
            textureCoordinate = aTextureCoord.xy;
        }
);

const std::string defaultFragmentShader = STRINGS(
        precision mediump float;
        uniform sampler2D inputTexture;
        varying vec2 textureCoordinate;

        void main() {
            gl_FragColor = texture2D(inputTexture, textureCoordinate.xy);
        }
);

/**
 * 滤镜
 */
class GLFilter{
    public:
        GLFilter();
        ~GLFilter();

        virtual void initProgram();

        virtual void initProgram(const char* vertexShader, const char* fragmentShader);

        virtual void destroyProgram();

        //更新视图
        virtual void updateViewPort();

        //绘制纹理
        virtual void drawTexture(GLuint texture, const float* vertex, const float* textureVertex, bool update = true);

        virtual void drawTexture(FrameBuffer* frameBuffer, GLuint texture, const float* vertex, const float* textureVertex);

        virtual void setTextureSize(int width, int height);

        //设置输出大小
        virtual void setDisplaySize(int width, int height);

        virtual void setTimeStamp(double time);

        //设置强度
        virtual void setIntensity(float intensity);

        virtual void setInitialized(bool init);

        virtual bool isInitialized();

    protected:
        //绑定attribute属性
        virtual void bindAttribute(const float* vertex, const float* textureVertex);

        virtual void bindTexture(GLuint texture);

        virtual void DrawBefore();

        virtual void DrawAfter();

        virtual void onDraw();

        virtual void unbindVertexAttribute();

        virtual void unbindTexture();

        virtual GLenum getTextureType();

    protected:
        bool initialized;
        int programHandle;                     //程序句柄
        int vertextHanlde;                     //顶点坐标句柄
        int textureHandle;                     //纹理坐标句柄
        int inputTextureHandle[MAX_TEXTURES];   //纹理句柄列表
        int nb_textures;                        //纹理数量
        int vertextCount = 4;                   //绘制顶点的个数
        double timeStamp;
        float intensity;                            //强度0.0~1.0 默认为1.0
        int textureWidth, textureHeight;
        int displayWidth, displayHeight;
};
#endif //MYPLAYER_GLFILTER_H