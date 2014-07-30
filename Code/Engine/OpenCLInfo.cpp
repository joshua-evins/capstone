#include "OpenCLInfo.h"
#include <fstream>
#include <iostream>

namespace Engine
{
	void OpenCLInfo::initialize(const char* kernelSourceFile)
	{
		VECTOR_CLASS<cl::Platform> platforms;
		cl::Platform::get(&platforms);

		cl::Platform& platform = platforms[0];

		cl_context_properties properties[] =
		{
			CL_GL_CONTEXT_KHR,
			((cl_context_properties)wglGetCurrentContext()),
			CL_WGL_HDC_KHR,
			(cl_context_properties)(wglGetCurrentDC()),
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)(platforms[0]()),
			0
		};
        
		initializeDevice(properties);

		context = cl::Context(CL_DEVICE_TYPE_GPU, properties);

		queue = cl::CommandQueue(context, device);

		initializeProgram(kernelSourceFile);
	}

	void OpenCLInfo::initializeDevice(cl_context_properties* properties)
	{
		cl_device_id interopDeviceId;

        int status;
        size_t deviceSize = 0;

        // Load extension function call
        clGetGLContextInfoKHR_fn glGetGLContextInfo_func = (clGetGLContextInfoKHR_fn)clGetExtensionFunctionAddress("clGetGLContextInfoKHR");

        // Ask for the CL device associated with the GL context
        status = glGetGLContextInfo_func( properties, 
                                    CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
                                    sizeof(cl_device_id), 
                                    &interopDeviceId, 
                                    &deviceSize);

#ifdef __CL_ENABLE_EXCEPTIONS
        if(deviceSize == 0) {
        throw cl::Error(1,"No GLGL devices found for current platform");
        }

        if(status != CL_SUCCESS) {
                throw cl::Error(1, "Could not get CLGL interop device for the current platform. Failure occured during call to clGetGLContextInfoKHR.");
        }
#else
		if(deviceSize == 0) {
			std::cout << "No GLGL devices found for current platform" << std::endl;
        }

		if(status != CL_SUCCESS) {
                std::cout << "Could not get CLGL interop device for the current platform. Failure occured during call to clGetGLContextInfoKHR." << std::endl;
        }
#endif

		device = cl::Device(interopDeviceId);
	}

	void OpenCLInfo::initializeProgram(const char* kernelSourceFile)
	{
		std::ifstream sourceFile(kernelSourceFile);
        std::string sourceCode(
            std::istreambuf_iterator<char>(sourceFile),
            (std::istreambuf_iterator<char>()));
        cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()+1));
 
        // Make program of the source code in the context
        program = cl::Program(context, source);

		VECTOR_CLASS<cl::Device> dev;

		dev.push_back(device);

#ifdef __CL_ENABLE_EXCEPTIONS
		try{
		program.build(dev);
		}catch(cl::Error error){
			if(error.err() == CL_BUILD_PROGRAM_FAILURE) {
                std::cout << "Build log:" << std::endl << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
            }
		}
#else
		program.build(dev);
		 std::cout << "Build log:" << std::endl << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
#endif
	
	}
}