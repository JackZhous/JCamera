// GL_OES_EGL_image_external 格式纹理输入滤镜，其中transformMatrix是SurfaceTexture的transformMatrix
attribute vec4 aPosition;
attribute vec4 aTextureCoord;

varying highp vec2 blurtexCoor[7];
uniform float xStep;
uniform float yStep;   //归一化后每个步长

void main(){
    gl_Position = aPosition;
    blurtexCoor[0] = aTextureCoord.xy;
    blurtexCoor[1] = aTextureCoord.xy + vec2(xStep, 0);
    blurtexCoor[2] = aTextureCoord.xy + vec2(-xStep, 0);
    blurtexCoor[3] = aTextureCoord.xy + vec2(0, yStep);
    blurtexCoor[4] = aTextureCoord.xy + vec2(0, yStep);
    blurtexCoor[5] = aTextureCoord.xy + vec2(xStep, yStep);
    blurtexCoor[6] = aTextureCoord.xy + vec2(-xStep, -yStep);
}


