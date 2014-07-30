#pragma once
#include <RendererWindow.h>
#include <Renderer.h>
#define __CL_ENABLE_EXCEPTIONS
#define __NO_STD_VECTOR // Use cl::vector instead of STL version
#include <CL\cl.hpp>
#include <OpenCLInfo.h>
#include <KernelWithRanges.h>

class Window_NB : public Engine::RendererWindow
{
public:
	Window_NB();
	virtual void initialize();
	virtual void inheritedUpdate();

	int numParticles;

	Engine::OpenCLInfo clInfo;

	Engine::Renderer::BufferedGeometryInfo* geo;
	Engine::Renderer::UniformInfo* facing;

	Engine::KernelWithRanges NBody;

	cl::BufferGL pos;
	cl::Buffer newPos;
	cl::BufferGL vel;

	void setupCL();
	void makeBuffers();
	void makeKernels();
	void setUnchangingKernelArgs();
};