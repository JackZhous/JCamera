//边框滤镜
precision mediump float;
varying highp vec2 textureCoordinate;
uniform sampler2D inputTexture;
uniform sampler2D bitmapTexture;

void main()
{
    vec4 color = texture2D(inputTexture,textureCoordinate);
    //透明白色背景图片点乘产生叠加效果 透明色（1，1，1）点乘对原rgb无影响，边框阴影小于1，点乘会更小，产生阴影
    //    vec3 finalColor = border.rgb * color.rgb;
    //黑色透明背景不能采用上面效果,用以下算法
    float y = 1.0 - textureCoordinate.y;
    //图片倒置的关系，需要倒向
    vec4 border = texture2D(bitmapTexture,vec2(textureCoordinate.x, y));
    color = 1.0 - ((1.0 - color) * (1.0 - border));
    gl_FragColor = color;
}