//
// Created by jackzhous on 2019/9/29.
//

#ifndef FFMPEGPLAYER_GLINPUTRGBAFILTER_H
#define FFMPEGPLAYER_GLINPUTRGBAFILTER_H


#include "GLInputFilter.h"

class GLInutRGBAFilter : public GLInputFilter{

public:
    GLInutRGBAFilter();
    ~GLInutRGBAFilter();

    void initProgram() override ;

    void initProgram(const char* vertexShader, const char* fragmentShader) override ;

    virtual GLboolean uploadTexture(Texture* texture) override ;

    virtual GLboolean renderTexture(Texture* texture, float *vertices, float* textureVertices) override ;

};

#endif //FFMPEGPLAYER_GLINPUTRGBAFILTER_H
