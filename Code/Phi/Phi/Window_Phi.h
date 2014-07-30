#pragma once
#include <RendererWindow.h>
#include <Renderer.h>
#define __CL_ENABLE_EXCEPTIONS
#define __NO_STD_VECTOR // Use cl::vector instead of STL version
#include <CL\cl.hpp>
#include <OpenCLInfo.h>
#include <KernelWithRanges.h>
#include "Circle.h"

class Window_Phi : public Engine::RendererWindow
{
public:
	int currFrame;
	int width;
	Window_Phi();
	virtual void initialize();
	virtual void inheritedUpdate();

	Engine::OpenCLInfo clInfo;

	Engine::KernelWithRanges stampCircle;

	cl::Buffer image;

	uchar* cpuBuffer;

	void setupCL();
	void makeBuffers();
	void makeKernels();
	void setUnchangingKernelArgs();

	void stampCircleToImage(float x, float y, float diameter, float lineWidth = 8.0f);
	void stampCircleObject(Circle* circle);
};

