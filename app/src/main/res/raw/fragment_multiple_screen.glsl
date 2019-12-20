//4分屏
precision mediump float;
varying highp vec2 textureCoordinate;
uniform sampler2D inputTexture;

void main()
{
    vec2 pos = vec2(0.0);
    if(textureCoordinate.y < 0.5 && textureCoordinate.x < 0.5){
        pos = vec2(textureCoordinate.x * 2.0, textureCoordinate.y * 2.0);
    }else if(textureCoordinate.y > 0.5 && textureCoordinate.x < 0.5){
        pos = vec2(textureCoordinate.x * 2.0, (textureCoordinate.y - 0.5)/0.5);
    }else if(textureCoordinate.y < 0.5 && textureCoordinate.x > 0.5){
        pos = vec2((textureCoordinate.x - 0.5)/0.5, textureCoordinate.y * 2.0);
    }else{
        pos = vec2((textureCoordinate.x - 0.5)/0.5, (textureCoordinate.y - 0.5)/0.5);
    }

    gl_FragColor = texture2D(inputTexture,pos);
}