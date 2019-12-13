// 上中下屏幕分割  上下高斯模糊 中间原图
precision highp float;
uniform sampler2D inputTexture; // 原始图像

varying highp vec2 blurtexCoor[7];
void main() {
    lowp vec4 color = vec4(0.0);
    if(blurtexCoor[0].y >= 0.25 && blurtexCoor[0].y <= 0.75){
        color = texture2D(inputTexture, blurtexCoor[0]);
    }else{

        color = texture2D(inputTexture, blurtexCoor[0]) * 0.3;
        color += texture2D(inputTexture, blurtexCoor[1]) * 0.15;
        color += texture2D(inputTexture, blurtexCoor[2]) * 0.15;
        color += texture2D(inputTexture, blurtexCoor[3]) * 0.11;
        color += texture2D(inputTexture, blurtexCoor[4]) * 0.11;
        color += texture2D(inputTexture, blurtexCoor[5]) * 0.09;
        color += texture2D(inputTexture, blurtexCoor[6]) * 0.09;
    }
    gl_FragColor = color;
}