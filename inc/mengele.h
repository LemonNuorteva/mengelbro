#pragma once

#include <inttypes.h>
#include <vector>

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

    Frame m_frame;
};