#pragma once

#include <array>
#include <inttypes.h>
#include <new>
#include <vector>
#include <cmath>

#include <iostream>

#include "params.h"

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
