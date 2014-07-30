#pragma once
#include <CL\cl.hpp>
#include "ExportHeader.h"

namespace Engine
{
	class ENGINE_SHARED OpenCLInfo
	{
	public:
		cl::Platform platform;
		cl::Device device;
		cl::CommandQueue queue;
		cl::Program program;
		cl::Context context;

		void initialize(const char* kernelSourceFile);
	private:
		void initializeDevice(cl_context_properties* properties);
		void initializeProgram(const char* kernelSourceFile);
	};
}