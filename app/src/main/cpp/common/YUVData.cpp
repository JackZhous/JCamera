//
// Created by jackzhous on 2019/12/30.
//

#include "YUVData.h"

YUVData::YUVData() : dataY(nullptr), dataU(nullptr), dataV(nullptr),
                    lineSizeV(0), lineSizeU(0), lineSizeV(0),
                    width(0), height(0){}

YUVData::YUVData(int width, int height) : width(width), height(height),
                                            dataY(new uint8_t(width*height)),
                                            dataU(new uint8_t(width*height / 4)),
                                            dataV(new uint8_t(width * height /4)),
                                            lineSizeY(width), lineSizeU(width/2), lineSizeV(width/2){}

YUVData::~YUVData() {
    release();
}

void YUVData::alloc(int width, int height) {
    if(dataV || dataU || dataY){
        release();
    }

    lineSizeY = width;
    lineSizeU = width /2;
    lineSizeV = width / 2;

    dataY = new uint8_t[width * height];
    dataU = new uint8_t[width * height / 4];
    dataV = new uint8_t[width * height / 4];
    this->width = width;
    this->height = height;
}


void YUVData::setData(uint8_t *data) {
    memcpy(dataY, data, (size_t)width*height);
    memcpy(dataU, data + width * height, (size_t)width*height / 4);
    memcpy(dataV, data + width * height*5/4, (size_t)width*height / 4);
}

void YUVData::release() {
    if(dataY != nullptr){
        delete [] dataY;
        dataY = nullptr;
    }

    if(dataU != nullptr){
        delete [] dataU;
        dataU = nullptr;
    }

    if(dataV != nullptr){
        delete [] dataV;
        dataV = nullptr;
    }
    lineSizeV = 0;
    lineSizeU = 0;
    lineSizeY = 0;
}

YUVData* YUVData::clone() {
    if(width <= 0 || height <= 0){
        return nullptr;
    }

    YUVData* p = new YUVData();
    p->alloc(width, height);
    memcpy(p->dataY, dataY, (size_t)width*height);
    memcpy(p->dataU, dataU, (size_t)width*height/4);
    memcpy(p->dataV, dataV, (size_t)width*height/4);
}
