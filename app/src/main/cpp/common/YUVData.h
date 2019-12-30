//
// Created by jackzhous on 2019/12/30.
//

#ifndef JCAMERA_YUVDATA_H
#define JCAMERA_YUVDATA_H


#include <cstdint>
#include <cstring>


class YUVData {
public:
    int width;
    int height;

    uint8_t *dataY;
    uint8_t *dataU;
    uint8_t *dataV;

    int lineSizeY;
    int lineSizeU;
    int lineSizeV;


public:
    YUVData();

    YUVData(int width, int height);

    virtual ~YUVData();

    void alloc(int width, int height);

    YUVData* clone();

    void setData(uint8_t *data);

    void release();
};


#endif //JCAMERA_YUVDATA_H
