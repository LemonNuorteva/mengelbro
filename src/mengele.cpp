#include "mengele.h"

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

const Frame& Mengele::calcFrame(const FrameParams& params)
{
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
    if (m_last == params)
    {
        return;
    }

    m_last = params;

    for (uint32_t i = 0; i < params.width; i++)
    {
        const real x0 = params.zoom * (params.x + ((real)i / 
            params.width * (0.47 + 2.0) - 2.0));

        const real y0 = params.zoom * (params.y +  ((real)fieldNum / 
            params.height * (1.12 + 1.12) - 1.12));

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