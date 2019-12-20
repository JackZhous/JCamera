//镜像
precision mediump float;
varying highp vec2 textureCoordinate;
uniform sampler2D inputTexture;


void main(){
    if(textureCoordinate.y < 0.5){
        gl_FragColor = texture2D(inputTexture,textureCoordinate);
    }else{
        float y = 1.0 - textureCoordinate.y;
        gl_FragColor = texture2D(inputTexture,vec2(textureCoordinate.x, y));
    }
}