// mandelbrot fractal in one api
#include "mandelbrot_oneapi.h"
#include <SFML/Graphics.hpp>
#include <sycl/sycl.hpp>

using namespace sycl;

// calculate the mandelbrot in sycl
void Mandelbrot::calculateMandelbrot(uint32_t* buffer_a)
{
	// create the buffer
	buffer<uint32_t, 1> buffer_a_d(
		buffer_a,
		range<1>(m_params.width * m_params.height)
	);

	const int width = this->m_params.width;
	const int height = this->m_params.height;
	const uint32_t maxIters = this->m_params.maxIters;

	const auto zoomX = m_params.zoom * m_params.zoomCur;
    const auto zoomY = m_params.zoom / m_params.zoomCur;

	const real x = m_params.x;
	const real y = m_params.y;


	// submit the command group
	q.submit([&](handler& cgh)
	{
		// get the accessor
		auto buffer_a_d_a = buffer_a_d.get_access<access::mode::write>(cgh);

		// calculate the mandelbrot
		cgh.parallel_for<class mandelbrot>(
			range<1>(width * height),
			[=](id<1> index)
		{
			// get the x and y coordinates
			// int x = index[0] % width;
			// int y = index[0] / width;

			// calculate the mandelbrot
			//float x0 = (x - width / 2.0f) * 4.0f / width;
			//float y0 = (y - height / 2.0f) * 4.0f / width;
			const real x0 = x + zoomX * (
				(real(index[0] % width) / real(width)) - 0.5
			);

			const real y0 = y + zoomY * (
				(real(index[0] / width) / real(height)) - 0.5
			);
			real x1 = 0.0f;
			real y1 = 0.0f;
			int iteration = 0;
			//int max_iteration = 10000;
			while (x1 * x1 + y1 * y1 <= 4 && iteration < maxIters)
			{
				real xtemp = x1 * x1 - y1 * y1 + x0;
				y1 = 2 * x1 * y1 + y0;
				x1 = xtemp;
				iteration++;
			}

			buffer_a_d_a[index] = iteration;
		});
	});


}

// mandelbrot constructor
Mandelbrot::Mandelbrot()
{
}

// mandelbrot destructor
Mandelbrot::~Mandelbrot()
{
}

// initialize the mandelbrot
void Mandelbrot::initMandelbrot(const FrameParams& params)
{
	m_params = params;
	if (m_params.height != m_last.height
		|| m_params.width != m_last.width)
	{
		if (buffer_a) delete buffer_a;
		buffer_a = new uint32_t[m_params.width * m_params.height];
	}
	m_last = m_params;
}

// update the mandelbrot
void Mandelbrot::updateMandelbrot()
{
	// calculate the mandelbrot in sycl
	calculateMandelbrot(buffer_a);

}

// run the mandelbrot
uint32_t* Mandelbrot::runMandelbrot()
{

	auto start = std::chrono::high_resolution_clock::now();

	//run 1000 iterations of the mandelbrot
	updateMandelbrot();

	// wait for queue
	q.wait();

	// end time
	auto end = std::chrono::high_resolution_clock::now();

	// print the time
	//std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

	// print the average time of one iteration
	//std::cout << "Average time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 100.0f << "ms" << std::endl;

	return buffer_a;
}
