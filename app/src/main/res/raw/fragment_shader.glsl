//引入扩展库
#extension GL_OES_EGL_image_external : require

precision mediump float;
varying vec2 textureCoordinate;
//samplerExternalOES代替贴图片时的sampler2D,作用就是和surfaceTexture配合进行纹理更新和格式转换
uniform samplerExternalOES inputTexture;
void main(){
    gl_FragColor = texture2D(inputTexture, textureCoordinate);
}
