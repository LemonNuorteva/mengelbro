/*
for each pixel (Px, Py) on the screen do
    x0 := scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.00, 0.47))
    y0 := scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1.12, 1.12))
    x := 0.0
    y := 0.0
    iteration := 0
    max_iteration := 1000
    while (x*x + y*y ≤ 2*2 AND iteration < max_iteration) do
        xtemp := x*x - y*y + x0
        y := 2*x*y + y0
        x := xtemp
        iteration := iteration + 1
    
    color := palette[iteration]
    plot(Px, Py, color)
*/
#include "mengele.h"

#include <iostream>

const Frame& Mengele::calcFrame(const FrameParams& params)
{
    if (m_last == params)
    {
        return m_frame;
    }

    m_last = params;

    m_frame.resize(params.width * params.height);

    #pragma omp parallel for
    for (uint32_t j = 0; j < params.height; j++)
    {
        calcField(j, params);
    }

    return m_frame;
}

void Mengele::calcField(
    const uint32_t fieldNum,
    const FrameParams& params
)
{
    const auto zoomX = params.zoom*params.zoomCur;
    const auto zoomY = params.zoom/params.zoomCur;

    for (uint32_t i = 0; i < params.width; i++)
    {
        const real x0 = params.x + zoomX * 
            ( (real(i) / params.width) - 0.5 );

        const real y0 = params.y + zoomY * 
            ( (real(fieldNum) / params.height) - 0.5 );

        uint32_t iterator = 0;
        real x{};
        real y{};

        while (x*x + y*y <= 4 && iterator < params.maxIters)
        {
            real xTemp = x*x - y*y + x0;
            y = 2*x*y + y0;
            x = xTemp;
            iterator++;
        }

        m_frame.at(fieldNum * params.width + i) = iterator;
    }
}

// Convolution ----------------------------------------------------
/*
const Frame Mengele::convolute(
    const int height,
    const int width,
    const Frame& frame, 
    const Conv& conv
)
{
    Frame out = Frame(height * width);

    for (int i = 0; i < height * width; i++)
    {
        out.at(i) = multiply(
            frame.at(i), 
            genConvKernel(
                height,
                width,
                frame,
                conv
            )
        );
    }

    return out;
}

real Mengele::multiply(
    const real x, 
    const Conv& convKern
)
{
    real accum = {};

    for (const auto& i : convKern)
    {
        for (const auto& j : i)
        {
            accum += x*j;
        }
    }

    return accum;
}

const Conv Mengele::genConvKernel(
    const int height,
    const int width,
    const Frame& frame,
    const Conv& conv
)
{
    Conv out = Conv(conv.size(), std::vector<real>(conv.at(0).size(), {}));

    const int convMiddleH = conv.size() / 2;
    const int convMiddleW = conv.at(0).size() / 2;

    for (int i = 0; i < out.size(); i++)
    {
        for (int j = 0; j < out.at(0).size(); j++)
        {
            const int frameW = j - convMiddleW;
            const int frameH = i - convMiddleH;

            if (hPos < 0 || hPos >= conv.size()
                || wPos < 0 || wPos >= conv.at(0).size()) 
            {
                continue;
            }
            else
            {
                out.at(i).at(j) = frame.at(
                    height * (i - convMiddleH) + j - convMiddleW
                );
            }
        }
    }

    return out;
}*/
