#include "Window_NB.h"
#include <iostream>

float floatRand()
{
	return (rand()%100)/100.0f;
}

Window_NB::Window_NB()
{
	numParticles = 10 * 1024;
}

void Window_NB::initialize()
{
	renderer->camera.position = glm::vec3(10.0f, 10.0f, 10.0f);
	renderer->camera.facing = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
	renderer->camera.MovementMultiplier = 2.0f;
	renderer->pointSize = 3;
	uint programID = renderer->shaderPrograms[
			renderer->addProgram(
				"Content_NB\\Speck.vsh",
				"Content_NB\\Speck.fsh"
				)
	].programID;
	geo = renderer->addBufferedGeometry(numParticles, programID, Engine::Renderer::POINT_ARRAY);
	geo->addUniform(Engine::Renderer::GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE, "transform", sizeof(glm::mat4), &glm::mat4());
	facing = geo->addUniform(Engine::Renderer::GLMVEC3, "camFacing", sizeof(glm::vec3), &glm::vec3(1.0f));

	setupCL();
}

void Window_NB::inheritedUpdate()
{
	facing->updateInfo(sizeof(glm::vec3), &renderer->camera.facing);

	cl::Device& device = clInfo.device;
	cl::CommandQueue& queue = clInfo.queue;

	clEnqueueAcquireGLObjects(queue(), 1, &pos(), 0, 0, 0);
	clEnqueueAcquireGLObjects(queue(), 1, &vel(), 0, 0, 0);
	queue.finish();

	NBody.kernel.setArg(2, pos);
	NBody.kernel.setArg(3, newPos);
	queue.enqueueNDRangeKernel(NBody.kernel, cl::NullRange, NBody.global, NBody.local, NULL, NULL);

	queue.enqueueCopyBuffer(newPos, pos, 0, 0, sizeof(glm::vec4)*numParticles);
	clEnqueueReleaseGLObjects(queue(), 1, &pos(), 0, 0, 0);
	clEnqueueReleaseGLObjects(queue(), 1, &vel(), 0, 0, 0);

}

void Window_NB::setupCL()
{
	clInfo.initialize("Content_NB\\Kernels\\NBody.cl");
	makeKernels();
	makeBuffers();
	setUnchangingKernelArgs();
}

void Window_NB::makeKernels()
{
	int err = 0;
	cl::Program& program = clInfo.program;
    
	NBody.kernel = cl::Kernel(program, "NBody", &err);
}

void Window_NB::setUnchangingKernelArgs()
{
	const cl::Device& device = clInfo.device;
	int localSize;
	size_t size;
	int err;

	NBody.global = cl::NDRange(numParticles);
	err = clGetKernelWorkGroupInfo(NBody.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	NBody.local = cl::NDRange(localSize);

	cl::Kernel& nbk = NBody.kernel;
	nbk.setArg(0, 0.0001f);
    nbk.setArg(1, 0.00001f);
	nbk.setArg(2, pos);
	nbk.setArg(3, newPos);
	nbk.setArg(4, vel);
	nbk.setArg(5, cl::__local(localSize * sizeof(glm::vec4)));
	
	
}

void Window_NB::makeBuffers()
{
	cl::CommandQueue& queue = clInfo.queue;
	cl::Context& gpuContext = clInfo.context;
	int err;

	glm::vec4* pos_ = new glm::vec4[numParticles];
	glm::vec4* newPos_ = new glm::vec4[numParticles];
	glm::vec4* vel_ = new glm::vec4[numParticles];

	float maxX = 10.0f;
	float maxY = 10.0f;
	float maxZ = 10.0f;
	
	for(int i=0; i<numParticles; i++)
	{
		pos_[i] = glm::vec4(floatRand()*maxX, floatRand()*maxY, floatRand()*maxZ, 10.0f);
		newPos_[i] = pos_[i];
		vel_[i] = glm::vec4(0.0f);
	}
	
	newPos = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, numParticles * sizeof(glm::vec4), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer newPos creation failed" );
	}

	queue.enqueueWriteBuffer(newPos, true, 0, sizeof(glm::vec4)*numParticles, newPos_, NULL, NULL);
	
	uint posID = geo->addAttribBuffer(4, GL_FLOAT, sizeof(glm::vec4), pos_, sizeof(glm::vec4)*numParticles, GL_DYNAMIC_DRAW);
	uint velID = geo->addAttribBuffer(4, GL_FLOAT, sizeof(glm::vec4), vel_, sizeof(glm::vec4)*numParticles, GL_DYNAMIC_DRAW);
	pos() = clCreateFromGLBuffer(gpuContext(), CL_MEM_READ_WRITE, posID, NULL);
	vel() = clCreateFromGLBuffer(gpuContext(), CL_MEM_READ_WRITE, velID, NULL);

	delete [] pos_;
	delete [] newPos_;
	delete [] vel_;
}

