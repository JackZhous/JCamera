//方框马赛克
precision mediump float;
varying highp vec2 textureCoordinate;
uniform sampler2D inputTexture;

uniform float perPixelWidth;
uniform float perPixelHeight;

//马赛克包含的像素个数
uniform float pixelCounts;

void main(){
    //马赛克宽高
    float width = pixelCounts * perPixelWidth;
    float height = pixelCounts * perPixelHeight;
    //floor向下取整
    vec2 pos = vec2(width * floor(textureCoordinate.x / width), height * floor(textureCoordinate.y / height));
    gl_FragColor = texture2D(inputTexture,pos);
}
