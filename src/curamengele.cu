#include "cudamengele.h"

#include <unistd.h>
#include <err.h>

__device__ uint32_t calc(
    const double r0,
	const double i0,
    const FrameParams& params
)
{
	uint32_t iterator = 0;
	real r{};
	real r2{};
	real i{};
	real i2{};

	while (r2 + i2 <= 4.0 && iterator < params.maxIters)
	{
		i = 2*x*i + i0;
		r = r2 - i2 + r0;
		r2 = r*r;
		i2 = i*i;
		iterator++;
	}

	return iterator;
}

__global__ void mandel_kernel(uint32_t *counts, double xmin, double ymin,
            double step, int max_iter, int dim, uint32_t *colors) {
    int pix_per_thread = dim * dim / (gridDim.x * blockDim.x);
    int tId = blockDim.x * blockIdx.x + threadIdx.x;
    int offset = pix_per_thread * tId;
    for (int i = offset; i < offset + pix_per_thread; i++){
        int x = i % dim;
        int y = i / dim;
        double cr = xmin + x * step;
        double ci = ymin + y * step;
        counts[y * dim + x]  = colors[mandel_double(cr, ci, max_iter)];
    }
    if (gridDim.x * blockDim.x * pix_per_thread < dim * dim
            && tId < (dim * dim) - (blockDim.x * gridDim.x)){
        int i = blockDim.x * gridDim.x * pix_per_thread + tId;
        int x = i % dim;
        int y = i / dim;
        double cr = xmin + x * step;
        double ci = ymin + y * step;
        counts[y * dim + x]  = colors[mandel_double(cr, ci, max_iter)];
    }

}

const Frame& CuMengele::calcFrame(
	const FrameParams& fraPar,
	Frame& frame
)
{
	cudaError_t err = cudaSuccess;

	static uint32_t *colors;
	uint32_t *dev_colors;

	const size_t color_size = (params.maxIters) * sizeof(uint32_t);
	colors = (uint32_t *) malloc(color_size);
	cudaMalloc((void**)&dev_colors, color_size);

	cudaMemcpy(dev_colors, colors, color_size, cudaMemcpyHostToDevice);
	free(colors);

	uint32_t *dev_counts = NULL;
    size_t img_size = dim * dim * sizeof(uint32_t);
    err = cudaMalloc(&dev_counts, img_size);
    checkErr(err, "Failed to allocate dev_counts");

	mandel_kernel<<<fraPar.height, fraPar.width>>>(
		
	);

}



