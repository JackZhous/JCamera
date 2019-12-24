//
// Created by jackzhous on 2019-09-16.
//

#ifndef MYPLAYER_COORDINATEUTILS_H
#define MYPLAYER_COORDINATEUTILS_H

//视频旋转角度
typedef enum {
    RORATE_NONE = 0,
    RORATE_90,
    RORATE_180,
    RORATE_270,
    RORATE_FLIP_VERTICAL,                   //竖直翻转
    RORATE_FLIP_HORIZONTAL                  //水平翻转
} RotationMode;

class CoordinateUtils{

public:
    static const float *getVertexCoordinates();

    static const float *getInputTextureCoordinates(const RotationMode &mode);

    static const float *getTextureCoordinates(const RotationMode &mode);

private:
    CoordinateUtils() = default;
    ~CoordinateUtils(){};
};

#endif //MYPLAYER_COORDINATEUTILS_H
