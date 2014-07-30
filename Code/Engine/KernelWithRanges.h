#pragma once
#include <CL\cl.hpp>
#include "ExportHeader.h"

namespace Engine
{

	struct ENGINE_SHARED KernelWithRanges
	{
	public:
		cl::Kernel kernel;
		cl::NDRange global;
		cl::NDRange local;
	};

}