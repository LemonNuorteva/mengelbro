#pragma once

#include <array>
#include <inttypes.h>
#include <new>
#include <vector>
#include <cmath>

#include <iostream>

using real = double;

struct Color
{
    real h, s, l;
};

using Frame = std::vector<uint32_t>;

struct FrameParams
{
    real x = 0.0, y = 0.0, zoom = 0.0, zoomCur = 1.0;
    int width = 0, height = 0;
    uint32_t maxIters = 0;

    bool operator==(const FrameParams& ot)
    {
        const double eps = 0.000000000000000000000001;

        const bool xIs = std::abs(this->x - ot.x) < eps;
        const bool yIs = std::abs(this->y - ot.y) < eps;
        const bool zoomIs = std::abs(this->zoom - ot.zoom) < eps;
        const bool zoomCurIs = std::abs(this->zoomCur - ot.zoomCur) < eps;

        return xIs && yIs && zoomIs && zoomCurIs
            && this->width == ot.width
            && this->height == ot.height
            && this->maxIters == ot.maxIters;
    }

    FrameParams& operator=(const FrameParams& ot) = default;
};

using Conv = std::vector<real>;

struct ConvParams
{
    int width = 0, height = 0;
};

class Mengele
{
public:

    Mengele(){};

    const Frame& calcFrame(const FrameParams& params);

    // static const Frame convolute(
    //     const int height,
    //     const int width,
    //     const Frame& frame,
    //     const Conv& conv
    // );

private:

    void calcField(
        const uint32_t fieldNum,
        const FrameParams& params
    );

    // static const Conv genConvKernel(
    //     const int height, 
    //     const int width,
    //     const unsigned frameIndex,
    //     const Frame& frame,
    //     const Conv& conv
    // );

    // static real multiply(const real x, const Conv& convKern);

    FrameParams m_last;

    alignas(std::hardware_constructive_interference_size)
        Frame m_frame;
};