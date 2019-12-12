// 上中下屏幕分割  上下高斯模糊 中间原图
precision highp float;
uniform sampler2D inputTexture; // 原始图像
varying vec2 textureCoordinate;

uniform float xLen;         // x轴总长度
uniform float yLen;         // y轴总长度
const int coreSize = 5;     //滤波器矩阵大小3X3
uniform float xStep;
uniform float yStep;   //归一化后每个步长

// 代入高斯正态分布函数计算出来的权重值
varying float kernel[25];
    // = (0.0947416, 0.118318, 0.0947416, 0.118318, 0.147761, 0.118318, 0.0947416, 0.118318, 0.0947416)
void main() {

    //计算当前y轴距离
    vec2 pos = (gl_FragCoord.xy / vec2(xLen, yLen));
    int index=0;
    vec4 color = texture2D(inputTexture, textureCoordinate);
    if(textureCoordinate.y >= 0.3 && textureCoordinate.y <= 0.7){
        gl_FragColor = color;
    }else{

        vec4 guass;
//        pos = pos / vec2(1.1, 1.1);
                for(int i = 0; i < coreSize; i++){
                    for(int j = 0; j < coreSize; j++){
                        vec4 myColor = texture2D(inputTexture, (pos + vec2(float(i-2)*xStep, float(j-2)*yStep)* vec2(1.1, 1.1)));
                        guass += myColor * kernel[index];
                        index++;
                    }
                }
        gl_FragColor = guass/0.981811;
    }
}