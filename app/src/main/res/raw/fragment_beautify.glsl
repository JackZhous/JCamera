precision mediump float;

varying mediump vec2 textureCoordinate;
uniform sampler2D inputTexture;
//uniform sampler2D inputTexture;
//步长
uniform vec2 singleStepOffset;
//美颜等级
uniform mediump float params;

const highp vec3 W = vec3(0.299,0.587,0.114);
vec2 blurCoordinates[20];


float hardLight(float color)
{
    if(color <= 0.5)
        color = color * color * 2.0;
    else
        color = 1.0 - ((1.0 - color)*(1.0 - color) * 2.0);
    return color;
}

void main(){
    //原图
    vec3 centralColor = texture2D(inputTexture, textureCoordinate).rgb;
    blurCoordinates[0] = textureCoordinate.xy + singleStepOffset * vec2(0.0, -10.0);
    blurCoordinates[1] = textureCoordinate.xy + singleStepOffset * vec2(0.0, 10.0);
    blurCoordinates[2] = textureCoordinate.xy + singleStepOffset * vec2(-10.0, 0.0);
    blurCoordinates[3] = textureCoordinate.xy + singleStepOffset * vec2(10.0, 0.0);
    blurCoordinates[4] = textureCoordinate.xy + singleStepOffset * vec2(5.0, -8.0);
    blurCoordinates[5] = textureCoordinate.xy + singleStepOffset * vec2(5.0, 8.0);
    blurCoordinates[6] = textureCoordinate.xy + singleStepOffset * vec2(-5.0, 8.0);
    blurCoordinates[7] = textureCoordinate.xy + singleStepOffset * vec2(-5.0, -8.0);
    blurCoordinates[8] = textureCoordinate.xy + singleStepOffset * vec2(8.0, -5.0);
    blurCoordinates[9] = textureCoordinate.xy + singleStepOffset * vec2(8.0, 5.0);
    blurCoordinates[10] = textureCoordinate.xy + singleStepOffset * vec2(-8.0, 5.0);
    blurCoordinates[11] = textureCoordinate.xy + singleStepOffset * vec2(-8.0, -5.0);
    blurCoordinates[12] = textureCoordinate.xy + singleStepOffset * vec2(0.0, -6.0);
    blurCoordinates[13] = textureCoordinate.xy + singleStepOffset * vec2(0.0, 6.0);
    blurCoordinates[14] = textureCoordinate.xy + singleStepOffset * vec2(6.0, 0.0);
    blurCoordinates[15] = textureCoordinate.xy + singleStepOffset * vec2(-6.0, 0.0);
    blurCoordinates[16] = textureCoordinate.xy + singleStepOffset * vec2(-4.0, -4.0);
    blurCoordinates[17] = textureCoordinate.xy + singleStepOffset * vec2(-4.0, 4.0);
    blurCoordinates[18] = textureCoordinate.xy + singleStepOffset * vec2(4.0, -4.0);
    blurCoordinates[19] = textureCoordinate.xy + singleStepOffset * vec2(4.0, 4.0);

    float sampleColor = centralColor.g * 20.0;
    sampleColor += texture2D(inputTexture, blurCoordinates[0]).g;
    sampleColor += texture2D(inputTexture, blurCoordinates[1]).g;
    sampleColor += texture2D(inputTexture, blurCoordinates[2]).g;
    sampleColor += texture2D(inputTexture, blurCoordinates[3]).g;
    sampleColor += texture2D(inputTexture, blurCoordinates[4]).g;
    sampleColor += texture2D(inputTexture, blurCoordinates[5]).g;
    sampleColor += texture2D(inputTexture, blurCoordinates[6]).g;
    sampleColor += texture2D(inputTexture, blurCoordinates[7]).g;
    sampleColor += texture2D(inputTexture, blurCoordinates[8]).g;
    sampleColor += texture2D(inputTexture, blurCoordinates[9]).g;
    sampleColor += texture2D(inputTexture, blurCoordinates[10]).g;
    sampleColor += texture2D(inputTexture, blurCoordinates[11]).g;
    sampleColor += texture2D(inputTexture, blurCoordinates[12]).g * 2.0;
    sampleColor += texture2D(inputTexture, blurCoordinates[13]).g * 2.0;
    sampleColor += texture2D(inputTexture, blurCoordinates[14]).g * 2.0;
    sampleColor += texture2D(inputTexture, blurCoordinates[15]).g * 2.0;
    sampleColor += texture2D(inputTexture, blurCoordinates[16]).g * 2.0;
    sampleColor += texture2D(inputTexture, blurCoordinates[17]).g * 2.0;
    sampleColor += texture2D(inputTexture, blurCoordinates[18]).g * 2.0;
    sampleColor += texture2D(inputTexture, blurCoordinates[19]).g * 2.0;

    sampleColor = sampleColor / 48.0;
    /**
    * sampleColor高斯模糊数据均圆，原图减去滤波后的，会使一些能量特别高的，也就是原图和滤波差额较大的才会看得清，这样减掉后才有值，几乎相等的最后值都很小变黑
    * 所以这个相减，只有原图中像素与周边像素相邻差距大的，减掉后才有比较大的值，显示才看得清；哪些像素符合这个条件，1. 物体轮廓和痘印，部分粗糙皮肤
    * 下面加0.5我觉得有两个原因：1.不加的话highPass值太小，导致图像几乎是黑色，看不清 2.后续会对这个反差加强，白的更白，黑的更黑
    */
    //
    float highPass = centralColor.g - sampleColor + 0.5;
    //将这个反差扩大
    for(int i = 0; i < 5;i++)
    {   ////白色越白值越大  黑色越黑值越小
        highPass = hardLight(highPass);
    }
    //点积 luminance为原图计算后的灰度值
    float luminance = dot(centralColor, W);
    //luminance的params次冪 第二个参数越小，alpha越小，后一步原图与高反差的减后值越大，加上原图后值更大，亮度更好，也就是美颜越好
    float alpha = pow(luminance, 0.1);
    //原图减去高反差higPass，高反差小于0.5的变得更小，原图减去更小的黑色高反差，会是正值，保留的是正值加上原图，黑色像素部分能量更大，如痘印黑色皮肤，能量大显示就偏白，美白效果
    //而大于0.5的白色，高反差highPass更大，原图减去后则是负值，原图加负值则能量变小，白色就偏暗
    vec3 smoothColor = centralColor + (centralColor-vec3(highPass))*alpha*0.1;
    //细节融合，显示的图像更真实 max是原图和平滑后图像比较，原图黑色皮肤部分smoothColor后变大，所以取smoothColor，而皮肤白色部分smoothColor较小，所以取原图
    //mix函数，如果美颜越高，则alpha越小，smoothColor.rgb * （1-a）权重更大，保留原色多一些，优化的图像max更小，加上就是补充细节
    gl_FragColor = vec4(mix(smoothColor.rgb, max(smoothColor, centralColor), alpha), 1.0);
//    gl_FragColor = vec4(highPass, highPass, highPass, 1.0);
}