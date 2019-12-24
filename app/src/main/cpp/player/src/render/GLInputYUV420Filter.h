//
// Created by jackzhous on 2019/9/10.
//

#ifndef MYPLAYER_GLINPUTYUV420FILTER_H
#define MYPLAYER_GLINPUTYUV420FILTER_H

#include "GLInputFilter.h"
#include "common/Config.h"
#include "GLFilter.h"


const std::string yuv420FragmentShader = STRINGS(
        precision mediump float;
        varying highp vec2 textureCoordinate;
        uniform lowp sampler2D inputTextureY;
        uniform lowp sampler2D inputTextureU;
        uniform lowp sampler2D inputTextureV;

        void main(){
            vec3 yuv;
            vec3 rgb;
            //texture2D 会使用inputTextureY采样器去纹理坐标下才有生成rgba颜色，但是这里会几个通道颜色都存到了r分量下去了，因为绑定纹理数据时使用GL_LUMINANCE格式，就会把几个分量放到r下去
            yuv.r = texture2D(inputTextureY, textureCoordinate).r - (16.0 / 255.0);
            yuv.g = texture2D(inputTextureU, textureCoordinate).r - 0.5;
            yuv.b = texture2D(inputTextureV, textureCoordinate).r - 0.5;
            rgb = mat3(1.164, 1.164, 1.164,
                       0.0, -0.213, 2.112,
                       1.793, -0.533, 0.0) * yuv;
//            rgb = mat3( 1.164,     1.164,     1.164,
//                  0.0,    -0.392,     2.017,
//                  1.596,    -0.813,       0.0) * yuv;
            gl_FragColor = vec4(rgb, 1.0);
        }
);


class GLInputYUV420Filter : public GLInputFilter{
    public:
        GLInputYUV420Filter();

        virtual ~GLInputYUV420Filter();

        void initProgram() override ;

        void initProgram(const char* vertexShader, const char* fragmentShader) override ;

        GLboolean uploadTexture(Texture* texture) override ;

        GLboolean renderTexture(Texture* texture, float *vertices, float* textureVertices) override ;
};


#endif //MYPLAYER_GLINPUTYUV420FILTER_H
