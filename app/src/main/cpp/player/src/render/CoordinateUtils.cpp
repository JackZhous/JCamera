//
// Created by jackzhous on 2019-09-16.
//

#include "CoordinateUtils.h"

static const float defaultVertices[] = {
        -1.0f, -1.0f,
        1.0f,  -1.0f,
        -1.0f, 1.0f,
        1.0f,  1.0f
};

/**
 * 以下视频旋转就是纹理的选择，4个纹理坐标点构成矩形后，
 * 依次角度进行旋转，旋转后每个点相邻点不变，第一个点是
 * 仍然是移动后的第一个点
 * none
 * 3------4                4-----2
 * |      |   右旋转90度    |     |
 * |      |                |     |
 * 1------2                3-----1
 * 每次点的顺序都是1-2-3-4，变化的是每个点的坐标变了
 */
static const float texture_vertices_none_input[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
};

//static const float texture_vertices_90_input[] = {
//        1.0f, 1.0f,
//        1.0f, 0.0f,
//        0.0f, 1.0f,
//        0.0f, 0.0f
//};

static const float texture_vertices_180_input[] = {
        1.0f, 0.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
};

static const float texture_vertices_270_input[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
};

static const float texture_vertices_flip_horizontal[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
};

static const float texture_vertices_flip_vertical[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
};

/**
 * 图像像素和纹理位置是倒置关系，所以以下和上面的坐标是倒置的，
 * 不然的话出来会是一幅倒立的图片
 */
static const float texture_vertices_none[] = {
        0.0f, 0.0f, // left,  bottom
        1.0f, 0.0f, // right, bottom
        0.0f, 1.0f, // left,  top
        1.0f, 1.0f, // right, top
};

//static const float texture_vertices_90[] = {
//        1.0f, 0.0f, // right, bottom
//        1.0f, 1.0f, // right, top
//        0.0f, 0.0f, // left,  bottom
//        0.0f, 1.0f, // left,  top
//};

static const float texture_vertices_180[] = {
        1.0f, 1.0f, // righ,  top
        0.0f, 1.0f, // left,  top
        1.0f, 0.0f, // right, bottom
        0.0f, 0.0f, // left,  bottom
};


static const float texture_vertices_270[] = {
        0.0f, 1.0f, // left,  top
        0.0f, 0.0f, // left,  bottom
        1.0f, 1.0f, // right, top
        1.0f, 0.0f, // right, bottom
};

static const float texture_vertices_90[] = {
        1.0f, 0.0f, // right, bottom
        1.0f, 1.0f, // right, top
        0.0f, 0.0f, // left,  bottom
        0.0f, 1.0f, // left,  top
};

static const float texture_vertices_90_input[] = {
        1.0f, 1.0f, // right, top
        1.0f, 0.0f, // right, bottom
        0.0f, 1.0f, // left,  top
        0.0f, 0.0f, // left,  bottom
};

const float* CoordinateUtils::getVertexCoordinates() {
    return defaultVertices;
}


const float* CoordinateUtils::getInputTextureCoordinates(const RotationMode &mode) {
    switch (mode){
        case RORATE_90:
            return texture_vertices_90_input;

        case RORATE_180:
            return texture_vertices_180_input;

        case RORATE_270:
            return texture_vertices_270_input;

        case RORATE_FLIP_HORIZONTAL:
            return texture_vertices_flip_horizontal;

        case RORATE_FLIP_VERTICAL:
            return texture_vertices_flip_vertical;

        default:
            return texture_vertices_none_input;
    }
}

const float* CoordinateUtils::getTextureCoordinates(const RotationMode &mode) {
    switch (mode){
        case RORATE_90:
            return texture_vertices_90;

        case RORATE_180:
            return texture_vertices_180;

        case RORATE_270:
            return texture_vertices_270;

        case RORATE_FLIP_HORIZONTAL:
            return texture_vertices_flip_horizontal;

        case RORATE_FLIP_VERTICAL:
            return texture_vertices_flip_vertical;

        default:
            return texture_vertices_none;
    }
}