#pragma once

#include <array>
#include <inttypes.h>
#include <new>
#include <vector>
#include <cmath>

#include <iostream>

#include "params.h"

struct MengeleParams
{
	const double zoomX = 0.0, zoomY = 0.0;
};


class CuMengele
{
public:

    CuMengele(){};

    static const Frame& calcFrame(
		const FrameParams& params,
		Frame& frame
	);

private:

    FrameParams m_last;
};
