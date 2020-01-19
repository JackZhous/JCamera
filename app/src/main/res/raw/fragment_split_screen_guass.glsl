// 上中下屏幕分割  上下高斯模糊 中间原图
precision highp float;
uniform sampler2D inputTexture; // 原始图像
varying highp vec2 blurtexCoor[13];
void main() {
    vec4 color = vec4(0.0);
    if(blurtexCoor[0].y < 0.7 && blurtexCoor[0].y > 0.3){
        color = texture2D(inputTexture, blurtexCoor[0]);
    }else{
        color = texture2D(inputTexture, blurtexCoor[0]) * 0.3;
        color += texture2D(inputTexture, blurtexCoor[1]) * 0.12;
        color += texture2D(inputTexture, blurtexCoor[2]) * 0.12;
        color += texture2D(inputTexture, blurtexCoor[3]) * 0.07;
        color += texture2D(inputTexture, blurtexCoor[4]) * 0.07;
        color += texture2D(inputTexture, blurtexCoor[5]) * 0.06;
        color += texture2D(inputTexture, blurtexCoor[6]) * 0.06;
        color += texture2D(inputTexture, blurtexCoor[7]) * 0.04;
        color += texture2D(inputTexture, blurtexCoor[8]) * 0.04;
        color += texture2D(inputTexture, blurtexCoor[9]) * 0.03;
        color += texture2D(inputTexture, blurtexCoor[10]) * 0.03;
        color += texture2D(inputTexture, blurtexCoor[11]) * 0.03;
        color += texture2D(inputTexture, blurtexCoor[12]) * 0.03;
    }
    gl_FragColor = color;
}