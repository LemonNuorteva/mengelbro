#pragma once

#include <inttypes.h>
#include <vector>
#include <cmath>

using real = double;

//using color = uint32_t;
struct Color
{
    real h, s, l;
};

using Frame = std::vector<uint32_t>;

struct FrameParams
{
    real x, y, zoom;
    int width, height;
    uint32_t maxIters;

    bool operator==(const FrameParams& ot)
    {
        const double eps = 0.000000001;

        const bool xis = std::abs(this->x - ot.x) < eps;
        const bool yis = std::abs(this->y - ot.y) < eps;
        const bool zoomis = std::abs(this->zoom - ot.zoom) < eps;

        return xis && yis && zoomis
            && this->width == ot.width
            && this->height == ot.height
            && this->maxIters == ot.maxIters;
    }

    FrameParams operator=(const FrameParams& ot)
    {
        return FrameParams{
            .x = ot.x,
            .y = ot.y,
            .zoom = ot.zoom,
            .width = ot.width,
            .height = ot.height,
            .maxIters = ot.maxIters,
        };
    }
};

class Mengele
{
public:

    Mengele()
    {}

    const Frame& calcFrame(const FrameParams& params);

private:

    void calcField(
        const uint32_t fieldNum,
        const FrameParams& params
    );

    FrameParams m_last;

    Frame m_frame;
};