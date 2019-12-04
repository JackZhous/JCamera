// GL_OES_EGL_image_external 格式纹理输入滤镜，其中transformMatrix是SurfaceTexture的transformMatrix
attribute vec4 aPosition;
attribute vec4 aTextureCoord;
uniform mat4 transformMatrix;

varying vec2 textureCoordinate;

void main(){
    gl_Position = aPosition;
    textureCoordinate = (transformMatrix * aTextureCoord).xy;
}
