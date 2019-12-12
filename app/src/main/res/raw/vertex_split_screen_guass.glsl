// GL_OES_EGL_image_external 格式纹理输入滤镜，其中transformMatrix是SurfaceTexture的transformMatrix
attribute vec4 aPosition;
attribute vec4 aTextureCoord;

varying vec2 textureCoordinate;
varying float kernel[25];

void main(){
kernel[0] = 0.002915; kernel[1] = 0.013064; kernel[2] = 0.021539;kernel[3] = 0.013064; kernel[4] = 0.002915;
kernel[5] = 0.013064; kernel[6] = 0.058550; kernel[7] = 0.096532;kernel[8] = 0.058550; kernel[9] = 0.013064;
kernel[10] = 0.021539; kernel[11] = 0.096532; kernel[12] = 0.159155;kernel[13] = 0.096532; kernel[14] = 0.021539;
    kernel[15] = 0.013064; kernel[16] = 0.058550; kernel[17] = 0.096532;kernel[18] = 0.058550; kernel[19] = 0.013064;
    kernel[20] = 0.002915; kernel[21] = 0.013064; kernel[22] = 0.021539;kernel[23] = 0.013064; kernel[24] = 0.002915;
    gl_Position = aPosition;
    textureCoordinate = aTextureCoord.xy;
}


