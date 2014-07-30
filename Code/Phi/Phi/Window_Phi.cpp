#include "Window_Phi.h"
#include <iostream>
#include <Qt\qrgb.h>

Window_Phi::Window_Phi()
{
	width = 2048;
	currFrame = 0;
}

void Window_Phi::initialize()
{
	Circle::diameters[0] = 1597.0f;
	Circle::diameters[1] = 987.0f;
	Circle::diameters[2] = 610.0f;
	Circle::diameters[3] = 377.0f;
	Circle::diameters[4] = 233.0f;
	Circle::diameters[5] = 144.0f;
	Circle::diameters[6] = 89.0f;
	Circle::diameters[7] = 55.0f;
	Circle::diameters[8] = 34.0f;
	Circle::diameters[9] = 21.0f;
	Circle::diameters[10] = 13.0f;
	for(int i=0; i<11; i++)
	{
		Circle::radii[i] = Circle::diameters[i]/2.0f;
	}
	setupCL();
}

void Window_Phi::inheritedUpdate()
{

}

void Window_Phi::setupCL()
{
	clInfo.initialize("Content_Phi\\Kernels\\StampCircle.cl");
	makeKernels();
	makeBuffers();
	setUnchangingKernelArgs();

	uchar* tempWid = new uchar[width*width*4];
	cl::CommandQueue& queue = clInfo.queue;
	Circle* seed = new Circle(width/2.0f, width/2.0f, (float)width);

	cpuBuffer = new uchar[width*width*4];

	// Stamping happens here, in initialize
	clock.frameTick();
	stampCircleObject(seed);
	queue.finish();
	std::cout << "time: " << clock.deltaTime() << std::endl;

	QString base("");
	base.append(QString("%1").arg(width));
	base.append(".png");

	queue.enqueueReadBuffer(image, true, 0, width*width*4*sizeof(uchar), tempWid, NULL, NULL);
	QImage qimg(tempWid, width, width, QImage::Format_ARGB32);
	qimg.save(base);
}

void Window_Phi::stampCircleObject(Circle* circle)
{
	stampCircleToImage(circle->center.x, circle->center.y, circle->diameter, 7.0f);
	if(circle->left != NULL)
		stampCircleObject(circle->left);
	if(circle->right != NULL)
		stampCircleObject(circle->right);
	if(circle->top != NULL)
		stampCircleObject(circle->top);
	if(circle->bottom != NULL)
		stampCircleObject(circle->bottom);
}

/*// GPU stamp
void Window_Phi::stampCircleToImage(float x, float y, float diameter, float lineWidth)
{
	lineWidth = 3.0f;
	cl::CommandQueue& queue = clInfo.queue;
	cl::Kernel& sck = stampCircle.kernel;

	sck.setArg(2, diameter);
	sck.setArg(3, lineWidth);
	sck.setArg(4, glm::vec2(x,y));
	
	queue.enqueueNDRangeKernel(stampCircle.kernel, cl::NullRange, stampCircle.global, stampCircle.local, NULL, NULL);
	//queue.finish();
	//uchar* tempWid = new uchar[width*width*4];
	//queue.enqueueReadBuffer(image, true, 0, width*width*4*sizeof(uchar), tempWid, NULL, NULL);
	//QImage qimg(tempWid, width, width, QImage::Format_ARGB32);
	//QString base("artStep");

	//base.append(QString("%1").arg(currFrame));
	//base.append(".png");
	//qimg.save(base);
	//delete [] tempWid;
	//currFrame++;
}
*/

/// CPU stamp
void Window_Phi::stampCircleToImage(float x, float y, float diameter, float lineWidth)
{
	lineWidth = 3.0f;
	float circleRadius = diameter/2.0f;
	int imageWidth = width;
	glm::vec2 center(512.0f,512.0f);
	for(int i=0; i<imageWidth*imageWidth; i++)
	{
		int gid = i;
		glm::vec2 pos(0.0f + (gid%imageWidth), 0.0f + (gid/imageWidth));

		glm::vec2 diff = center - pos;
		float distance = sqrt(diff.x*diff.x + diff.y*diff.y);
		if(distance<circleRadius && distance>circleRadius-lineWidth)
		{
			//0 and lineWidth give 1
			float valF = 255.0f * sin(3.14159f * (circleRadius-distance)/lineWidth);
			valF += 255.0f * sin(3.14159f * (circleRadius-distance)/lineWidth);
			valF += 255.0f * sin(3.14159f * (circleRadius-distance)/lineWidth);
			valF += 255.0f * sin(3.14159f * (circleRadius-distance)/lineWidth);
			valF /= 4;

			valF = (valF>0.0f)? valF : -valF;
			uchar val = (char)(valF);
			val = (val>cpuBuffer[gid*4]) ? val : cpuBuffer[gid*4];
			cpuBuffer[gid*4] = val;
			cpuBuffer[gid*4 + 1] = val;
			cpuBuffer[gid*4 + 2] = val;
			cpuBuffer[gid*4 + 3] = 255;
		}
	}
}


void Window_Phi::makeBuffers()
{
	cl::CommandQueue& queue = clInfo.queue;
	cl::Context& gpuContext = clInfo.context;
	int err;
	char* image_ = new char[width*width*4];
	for(int i=0; i<width*width*4; i++)
	{
		image_[i] = (i%4==3) ? 255 : 0;
	}

	image = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, width*width*sizeof(char)*4, NULL, &err );
	queue.enqueueWriteBuffer(image, true, 0, width*width*sizeof(char)*4, image_, NULL, NULL);
	delete [] image_;
}

void Window_Phi::makeKernels()
{
	int err = 0;
	cl::Program& program = clInfo.program;
    
	stampCircle.kernel = cl::Kernel(program, "StampCircle", &err);
}

void Window_Phi::setUnchangingKernelArgs()
{
	const cl::Device& device = clInfo.device;
	int localSize;
	size_t size;
	int err;

	stampCircle.global = cl::NDRange(width*width);
	err = clGetKernelWorkGroupInfo(stampCircle.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	stampCircle.local = cl::NDRange(localSize);
	
	cl::Kernel& sck = stampCircle.kernel;
	sck.setArg(0, image);
    sck.setArg(1, width);
	sck.setArg(2, 1024.0f);
	sck.setArg(3, 8.0f);
	sck.setArg(4, glm::vec2(512.0f,512.0f));
}