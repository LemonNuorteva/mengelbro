#pragma once

#include <CL/sycl.hpp>

#include "params.h"

using namespace cl::sycl;

// define class mandelbrot
class Mandelbrot
{
public:

	// constructor
	Mandelbrot();

	// destructor
	~Mandelbrot();

	// initialize the mandelbrot
	void initMandelbrot(const FrameParams& params);

	// run the mandelbrot
	uint32_t* runMandelbrot();

	// calculate the mandelbrot
	void calculateMandelbrot(uint32_t* buffer_a);

	// update the mandelbrot
	void updateMandelbrot();

	// save to file
	// void saveToFile(std::filesystem::path path);

private:

	FrameParams m_last;
	FrameParams m_params;
	// buffer
	//int width, height;
	uint32_t* buffer_a = nullptr;
	queue q;
};
