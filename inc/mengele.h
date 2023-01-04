#pragma once

#include <array>
#include <inttypes.h>
#include <vector>
#include <cmath>

#include <iostream>

using real = double;

struct Color
{
    real h, s, l;
};

using Frame = std::vector<uint32_t>;
using Conv = std::vector<std::vector<real>>;

struct FrameParams
{
    real x = 0, y = 0, zoom = 0;
    int width = 0, height = 0;
    uint32_t maxIters = 0;

    bool operator==(const FrameParams& ot)
    {
        const double eps = 0.000000000000000000000001;

        const bool xis = std::abs(this->x - ot.x) < eps;
        const bool yis = std::abs(this->y - ot.y) < eps;
        const bool zoomis = std::abs(this->zoom - ot.zoom) < eps;

        return xis && yis && zoomis
            && this->width == ot.width
            && this->height == ot.height
            && this->maxIters == ot.maxIters;
    }

    FrameParams& operator=(const FrameParams& ot) = default;
};

class Mengele
{
public:

    Mengele(){};

    const Frame& calcFrame(const FrameParams& params);

    static const Frame convolute(
        const int height,
        const int width,
        const Frame& frame,
        const Conv& conv
    );

private:

    void calcField(
        const uint32_t fieldNum,
        const FrameParams& params
    );

    static const Conv genConvKernel(
        const int x, 
        const int y,
        const Frame& frame,
        const Conv& conv
    );

    static real multiply(const real x, const Conv& convKern);

    FrameParams m_last;

    Frame m_frame;
};