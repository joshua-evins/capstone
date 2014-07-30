#pragma once
#include <RendererWindow.h>
#include <Renderer.h>
#define __NO_STD_VECTOR // Use cl::vector instead of STL version
#define __CL_ENABLE_EXCEPTIONS
#include <utility>
#include <iostream>
#include <string>
#include <CL\cl.hpp>
#include <OpenCLInfo.h>
#include <KernelWithRanges.h>

class Window_SPH : public Engine::RendererWindow
{
public:
	Window_SPH();
	virtual void initialize();
	virtual void inheritedUpdate();
	
	Engine::OpenCLInfo clInfo;

	Engine::Renderer::UniformInfo* meshFacing;

	cl::Buffer position;
	cl::Buffer velocity;
	cl::Buffer acceleration;
	cl::Buffer lifetimes;
	cl::Buffer sortedLifetimes;
	
	cl::Buffer particleIndex;
	cl::Buffer particleIndex2;
	cl::Buffer shiftBuff;
	cl::Buffer workGroupOneCounts;
	cl::Buffer totalOnes;

	cl::Buffer sortedPosition;
	cl::Buffer sortedVelocity;
	cl::Buffer gridCellIndex;
	cl::Buffer gridCellIndexFixedUp;
	cl::Buffer neighborMap;
	cl::Buffer pressure;
	cl::Buffer rho;
	cl::Buffer rhoInv;

	cl::Buffer debugNewPos;

	
	Engine::KernelWithRanges hashParticles;
	Engine::KernelWithRanges radixClearBuffers;
	Engine::KernelWithRanges radixFillShiftBuffer;
	Engine::KernelWithRanges radixParseShiftBuffer;
	Engine::KernelWithRanges radixFillParticleIndexOut;
	Engine::KernelWithRanges sortPostPass;
	Engine::KernelWithRanges indexx;
	Engine::KernelWithRanges indexPostPass;
	Engine::KernelWithRanges findNeighbors;
	Engine::KernelWithRanges computeDensityPressure;
	Engine::KernelWithRanges computeAcceleration;
	Engine::KernelWithRanges integrate;
	Engine::KernelWithRanges clearBuffers;
	Engine::KernelWithRanges cycleParticles;

	int xLayers;
	int zLayers;
	float xDim;
	float zDim;

	glm::vec4 sourceVelocity;
	float endSize;
	float endMinX;
	float endMinZ;

	Engine::Renderer::BufferedGeometryInfo* geo;
	Engine::Renderer::UniformInfo* facing;



	void handleKeyInput();
	void setupCL();
	void makeBuffers();
	void makeKernels();
	void setUnchangingKernelArgs();

	void sortParticleIndex();
};