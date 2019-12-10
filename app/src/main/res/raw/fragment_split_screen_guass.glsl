// 上中下屏幕分割  上下高斯模糊 中间原图
precision mediump float;
uniform sampler2D inputTexture; // 原始图像
varying vec2 textureCoordinate;

uniform float xLen;  // Y分割距离
uniform float yLen;         // y轴总长度
const int coreSize = 3;     //滤波器矩阵大小3X3
uniform float xStep;
uniform float yStep;   //归一化后每个步长

// 代入高斯正态分布函数计算出来的权重值
float kernel[9];
// = (0.0947416, 0.118318, 0.0947416, 0.118318, 0.147761, 0.118318, 0.0947416, 0.118318, 0.0947416)



void main() {
kernel[0] = 0.0947416; kernel[1] = 0.118318; kernel[2] = 0.0947416;
kernel[3] = 0.118318; kernel[4] = 0.147761; kernel[5] = 0.118318;
kernel[6] = 0.0947416; kernel[7] = 0.118318; kernel[8] = 0.0947416;
    //计算当前y轴距离
    vec2 pos = (gl_FragCoord.xy / vec2(xLen, yLen));

    vec4 color;
    //中间图像不变
    if(pos.y >= 0.3 && pos.y <= 0.7){
        color = texture2D(inputTexture, textureCoordinate);

    //上下高斯模糊处理
    }else{
        int index=0;
        for(int i = 0; i < coreSize; i++){
            for(int j = 0; j < coreSize; j++){
                vec4 myColor = texture2D(inputTexture, pos + vec2(float(i-1) * xStep, float(j-1) * yStep));
                if(index == 0){
                    color += myColor * kernel[0];
                }else if(index == 1){
                    color += myColor * kernel[1];
                }else if(index == 2){
                    color += myColor * kernel[2];
                }else if(index == 3){
                    color += myColor * kernel[3];
                }else if(index == 4){
                    color += myColor * kernel[4];
                }else if(index == 5){
                    color += myColor * kernel[5];
                }else if(index == 6){
                    color += myColor * kernel[6];
                }else if(index == 7){
                    color += myColor * kernel[7];
                }else if(index == 8){
                    color += myColor * kernel[8];
                }
                index++;
            }
        }
    }
    gl_FragColor = color;
}