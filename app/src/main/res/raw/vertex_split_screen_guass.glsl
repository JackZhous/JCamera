// GL_OES_EGL_image_external 格式纹理输入滤镜，其中transformMatrix是SurfaceTexture的transformMatrix
attribute vec4 aPosition;
attribute vec4 aTextureCoord;

varying highp vec2 blurtexCoor[13];
uniform float xStep;
uniform float yStep;   //归一化后每个步长

void main(){
    gl_Position = aPosition;
    blurtexCoor[0] = aTextureCoord.xy;
    //乘2是因为步长太短  像素可能没什么变化
    blurtexCoor[1] = aTextureCoord.xy + vec2(xStep, 0.0);
    blurtexCoor[2] = aTextureCoord.xy + vec2(-xStep, 0.0);
    blurtexCoor[3] = aTextureCoord.xy + vec2(0.0, yStep);
    blurtexCoor[4] = aTextureCoord.xy + vec2(0.0, yStep);
    blurtexCoor[5] = aTextureCoord.xy + vec2(xStep, yStep);
    blurtexCoor[6] = aTextureCoord.xy + vec2(-xStep, -yStep);
    blurtexCoor[7] = aTextureCoord.xy + vec2(xStep*2.0, 0.0);
    blurtexCoor[8] = aTextureCoord.xy + vec2(-xStep*2.0, 0.0);
    blurtexCoor[9] = aTextureCoord.xy + vec2(0.0, yStep*2.0);
    blurtexCoor[10] = aTextureCoord.xy + vec2(0.0, yStep*2.0);
    blurtexCoor[11] = aTextureCoord.xy + vec2(xStep*2.0, yStep*2.0);
    blurtexCoor[12] = aTextureCoord.xy + vec2(-xStep*2.0, -yStep*2.0);
}


