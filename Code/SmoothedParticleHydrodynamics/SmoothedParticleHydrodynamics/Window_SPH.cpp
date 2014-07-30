#pragma OPENCL EXTENSION CL_KHR_gl_sharing : enable
#include "Window_SPH.h"
#include <glm\gtx\transform.hpp>

#define PARTICLE_COUNT ( 8 * 1024 )
#define NEIGHBOR_COUNT 32

#ifndef M_PI
#define M_PI 3.1415927f
#endif

#define XMIN 0
#define XMAX 50
#define YMIN 0
#define YMAX 50
#define ZMIN 0
#define ZMAX 100

const float xmin = XMIN;
const float xmax = XMAX;
const float ymin = YMIN;
const float ymax = YMAX;
const float zmin = ZMIN;
const float zmax = ZMAX;

const float rho0 = 1000.0f;
const float stiffness = 0.75f;
const float h = 3.34f;
const float hashGridCellSize = 2.0f * h;
const float hashGridCellSizeInv = 1.0f / hashGridCellSize;
const float mass = 0.0015f;
const float simulationScale = 0.004f;
const float simulationScaleInv = 1.0f / simulationScale;
const float mu = 1.0f;
const float timeStep = 0.003f;
const int NK = NEIGHBOR_COUNT * PARTICLE_COUNT;
const float CFLLimit = 100.0f;
const float damping = 0.75f;

const float Wpoly6Coefficient = 315.0f / ( 64.0f * M_PI * pow( h * simulationScale, 9.0f ) );
const float gradWspikyCoefficient= -45.0f / ( M_PI * pow( h * simulationScale, 6.0f ) );
const float del2WviscosityCoefficient = -gradWspikyCoefficient;

const float gravity_x = 0.0f;
const float gravity_y = -9.8f;
const float gravity_z = 0.0f;

int gridCellsX;
int gridCellsY;
int gridCellsZ;
int gridCellCount;

float floatRand()
{
	int maxi = 32767;
	return (rand()%maxi)/((float)(maxi));
}

Window_SPH::Window_SPH() : sourceVelocity(1.0f, 0.0f, 0.001f, 0.0f)
{
	gridCellsX = (int)( ( XMAX - XMIN ) / h ) + 1;
	gridCellsY = (int)( ( YMAX - YMIN ) / h ) + 1;
	gridCellsZ = (int)( ( ZMAX - ZMIN ) / h ) + 1;
	gridCellCount = gridCellsX * gridCellsY * gridCellsZ;
	endSize = 10.0f;
	endMinX = 40.0f;
	endMinZ = 45.0f;

	xLayers = int(std::sqrt((float)PARTICLE_COUNT)/4) + 1;
	zLayers = 2*xLayers;
	xDim = ((float)XMAX)/xLayers;
	zDim = ((float)ZMAX)/zLayers;
}

void Window_SPH::initialize()
{
	renderer->camera.position = glm::vec3(102.297f, 38.13f, 123.656f);
	renderer->camera.facing = glm::normalize(glm::vec3(-0.724946f, -0.20581f, -0.657341f));
	renderer->camera.MovementMultiplier = 8.0f;
	renderer->pointSize = 3;

	uint programID = renderer->shaderPrograms[
			renderer->addProgram(
				"Content_SPH\\SPH_Speck.vsh",
				"Content_SPH\\SPH_Speck.fsh"
				)
	].programID;
	geo = renderer->addBufferedGeometry(PARTICLE_COUNT, programID, Engine::Renderer::POINT_ARRAY);
	geo->addUniform(Engine::Renderer::GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE, "transform", sizeof(glm::mat4), &glm::mat4());
	facing = geo->addUniform(Engine::Renderer::GLMVEC3, "camFacing", sizeof(glm::vec3), &glm::vec3(1.0f));


	setupCL();
}

int myCompare(const void * v1, const void * v2)
{
	int * f1 = (int *)v1;
	int * f2 = (int *)v2;
	if( f1[ 0 ] < f2[ 0 ] ) return -1;
	if( f1[ 0 ] > f2[ 0 ] ) return 1;
	return 0;
}

void Window_SPH::handleKeyInput()
{
	if(GetAsyncKeyState(Qt::Key::Key_Q)) // Q
	{
		sourceVelocity = glm::rotate(1.0f, 0.0f, 1.0f, 0.0f) * sourceVelocity;
		cycleParticles.kernel.setArg(8,sourceVelocity);
	}
	if(GetAsyncKeyState(Qt::Key::Key_E)) // E
	{
		sourceVelocity = glm::rotate(-1.0f, 0.0f, 1.0f, 0.0f) * sourceVelocity;
		cycleParticles.kernel.setArg(8,sourceVelocity);
	}
	float endSpeed = 0.2f;
	if(GetAsyncKeyState(Qt::Key::Key_I)) // I
	{
		endMinX += endSpeed;
		endMinX = std::min(endMinX, XMAX - endSize);
		cycleParticles.kernel.setArg(10,endMinX);
	}
	if(GetAsyncKeyState(Qt::Key::Key_K)) // K
	{
		endMinX -= endSpeed;
		endMinX = std::max(endMinX, (float)XMIN);
		cycleParticles.kernel.setArg(10,endMinX);
	}
	if(GetAsyncKeyState(Qt::Key::Key_J)) // J
	{
		endMinZ -= endSpeed;
		endMinZ = std::max(endMinZ, (float)ZMIN);
		cycleParticles.kernel.setArg(11,endMinZ);
	}
	if(GetAsyncKeyState(Qt::Key::Key_L)) // L
	{
		endMinZ += endSpeed;
		endMinZ = std::min(endMinZ, ZMAX - endSize);
		cycleParticles.kernel.setArg(11,endMinZ);
	}
}

void Window_SPH::inheritedUpdate()
{
	if(initialized)
	{
		handleKeyInput();
		//try{
		cl::Device& device = clInfo.device;
		cl::CommandQueue& queue = clInfo.queue;

		clEnqueueAcquireGLObjects(queue(), 1, &position(), 0, 0, 0);
		clEnqueueAcquireGLObjects(queue(), 1, &lifetimes(), 0, 0, 0);
		queue.finish();
		

		//clock.frameTick();
		queue.enqueueNDRangeKernel(clearBuffers.kernel, cl::NullRange, clearBuffers.global, clearBuffers.local, NULL, NULL);
		//queue.finish();
		//std::cout << "clear buffers: " << clock.deltaTime() << std::endl;


		//clock.frameTick();
		queue.enqueueNDRangeKernel(hashParticles.kernel, cl::NullRange, hashParticles.global, hashParticles.local, NULL, NULL );
		//queue.finish();
		//std::cout << "hash particles: " << clock.deltaTime() << std::endl;

		

		/*sort
		clock.frameTick();
		int * _particleIndex = new int[ PARTICLE_COUNT * 2 ];
		queue.enqueueReadBuffer( particleIndex, CL_TRUE, 0, PARTICLE_COUNT * 2 * sizeof( int ), _particleIndex );
								//queue.finish();
		
		qsort( _particleIndex, PARTICLE_COUNT, 2 * sizeof( int ), myCompare );
		
		queue.enqueueWriteBuffer( particleIndex, CL_TRUE, 0, PARTICLE_COUNT * 2 * sizeof( int ), _particleIndex );
		queue.finish();
		std::cout << clock.deltaTime() << std::endl;
								//queue.finish();
		 */// endsort
		
		
		//queue.finish();
		//clock.frameTick();
			sortParticleIndex();
		//queue.finish();
		//std::cout << "radix sort: " << clock.deltaTime() << std::endl;


		//clock.frameTick();
		queue.enqueueNDRangeKernel(sortPostPass.kernel, cl::NullRange, sortPostPass.global, sortPostPass.local, NULL, NULL);
		//queue.finish();
		//std::cout << "sort post pass: " << clock.deltaTime() << std::endl;


		//clock.frameTick();
		queue.enqueueNDRangeKernel(indexx.kernel, cl::NullRange, indexx.global, cl::NDRange(256), NULL, NULL);
		//queue.finish();
		//std::cout << "indexx: " << clock.deltaTime() << std::endl;


		//clock.frameTick();
		queue.enqueueNDRangeKernel(indexPostPass.kernel, cl::NullRange, indexPostPass.global, cl::NDRange(256), NULL, NULL);
		//queue.finish();
		//std::cout << "index post pass: " << clock.deltaTime() << std::endl;

		

		//clock.frameTick();
		queue.enqueueNDRangeKernel(findNeighbors.kernel, cl::NullRange, findNeighbors.global, findNeighbors.local, NULL, NULL);
		//queue.finish();
		//std::cout << "find neighbors: " << clock.deltaTime() << std::endl;

		//uint* _gcifu = new uint[gridCellCount+1];
		//uint* _particleIndex2 = new uint[PARTICLE_COUNT*2];
		//float* _nm = new float[PARTICLE_COUNT*NEIGHBOR_COUNT*2];

		//queue.enqueueReadBuffer(gridCellIndexFixedUp, true, 0, gridCellCount+1 * sizeof(uint), _gcifu, NULL, NULL);
		//queue.enqueueReadBuffer(particleIndex, true, 0, PARTICLE_COUNT*2*sizeof(uint), _particleIndex2, NULL, NULL);
		//queue.enqueueReadBuffer(neighborMap, true, 0, PARTICLE_COUNT*NEIGHBOR_COUNT*2*sizeof(float), _nm, NULL, NULL);

		//clock.frameTick();
		queue.enqueueNDRangeKernel(computeDensityPressure.kernel, cl::NullRange, computeDensityPressure.global, computeDensityPressure.local, NULL, NULL );
		//queue.finish();
		//std::cout << "compute density pressure: " << clock.deltaTime() << std::endl;


		//clock.frameTick();
		queue.enqueueNDRangeKernel(computeAcceleration.kernel, cl::NullRange, computeAcceleration.global, cl::NDRange(512), NULL, NULL );
		//queue.finish();
		//std::cout << "compute acceleration: " << clock.deltaTime() << std::endl;


		//clock.frameTick();
		queue.enqueueNDRangeKernel(integrate.kernel, cl::NullRange, integrate.global, integrate.local, NULL, NULL );
		//queue.finish();
		//std::cout << "integrate: " << clock.deltaTime() << std::endl;

		queue.enqueueNDRangeKernel(cycleParticles.kernel, cl::NullRange, cycleParticles.global, cycleParticles.local, NULL, NULL);

		clEnqueueReleaseGLObjects(queue(), 1, &position(), 0, 0, 0);
		clEnqueueReleaseGLObjects(queue(), 1, &lifetimes(), 0, 0, 0);

		glm::vec4* positions = new glm::vec4[PARTICLE_COUNT];


		facing->updateInfo(sizeof(glm::vec3), &renderer->camera.facing);
//		}catch(cl::Error e){
	//	std::cout << "Build log:" << std::endl << e.err() << std::endl;
	//}
	}
}

void mockK2(int* _shiftBuffer)
{
	int* result = new int[PARTICLE_COUNT];
	uint totalOnes = 0;
	for(int tally = 0; tally<PARTICLE_COUNT; tally++)
	{
		totalOnes += _shiftBuffer[tally];
	}

	for(int id=0; id<PARTICLE_COUNT/8; id++)
	{
		int gid = id*8;
		int onesToLeft = 0;
		for(int i=0; i<gid; i++)
		{
			onesToLeft += _shiftBuffer[i];
		}
		int totalZeroes = PARTICLE_COUNT - totalOnes;
		int zeroesToLeft = gid - onesToLeft;
		int zeroesOnMyIndex = 1 - _shiftBuffer[gid];
		int zeroesToRight = totalZeroes - zeroesToLeft - zeroesOnMyIndex;
		zeroesToRight++;
		zeroesToRight -= _shiftBuffer[gid];

		int oz[2];
		oz[0] = -onesToLeft;
		oz[1] = zeroesToRight;
		
		int tempgid = gid;

		for(int i=0; i<8; i++)
		{
			int a = _shiftBuffer[tempgid];
			int b = (a+1)%2;
			result[tempgid++ + oz[a]] = tempgid;
			oz[b]--;
		}
	}
	totalOnes = totalOnes + 1 - 1;
}

void Window_SPH::sortParticleIndex()
{
	cl::CommandQueue& queue = clInfo.queue;
	//queue.finish();
	uint numLoops = 0;
	uint numP = PARTICLE_COUNT;
	while(numP > 0)
	{
		numP >>= 1;
		numLoops++;
	}
	// even values of numLoops avoid the need for buffer swapping elsewhere, so for now make it even
	numLoops += numLoops%2;
	uint mask = 1;
	

	cl::Kernel& shiftBufferK = radixFillShiftBuffer.kernel;
	cl::Kernel& particleIndexOutK = radixFillParticleIndexOut.kernel;
	
	//int * _shiftBuffer = new int[ PARTICLE_COUNT];
	//uint * _debugNewPos = new uint[ PARTICLE_COUNT];
	//uint * _particleIndex = new uint[ PARTICLE_COUNT * 2 ];
	//uint * _particleIndex2 = new uint[ PARTICLE_COUNT * 2 ];
	//uint* _oneCounts = new uint[ 64 ];
	//uint* _totalOnes = new uint[2];

	int err;
	//float k1Elapsed = 0.0f;
	//float k2Elapsed = 0.0f;
	for(uint i=0; i<numLoops; i++)
	{
		//clock.frameTick();
		err = queue.enqueueNDRangeKernel(radixClearBuffers.kernel, cl::NullRange, radixClearBuffers.global, radixClearBuffers.local, NULL, NULL );
		//queue.finish();
		//k1Elapsed += clock.deltaTime();
		if(i%2 == 0)
		{
			//clock.frameTick();

			shiftBufferK.setArg(0, particleIndex);
			shiftBufferK.setArg(3, mask);
			err = queue.enqueueNDRangeKernel(shiftBufferK, cl::NullRange, radixFillShiftBuffer.global, radixFillShiftBuffer.local, NULL, NULL );
			//queue.finish();
			//std::cout << "k1 " << clock.deltaTime() << std::endl;
			//k1Elapsed += clock.deltaTime();
			
			
			//clock.frameTick();
			//err = queue.enqueueNDRangeKernel(radixParseShiftBuffer.kernel, cl::NullRange, radixParseShiftBuffer.global, radixParseShiftBuffer.local, NULL, NULL );


			particleIndexOutK.setArg(0, particleIndex);
			particleIndexOutK.setArg(1, particleIndex2);
			err = queue.enqueueNDRangeKernel(particleIndexOutK, cl::NullRange, radixFillParticleIndexOut.global, radixFillParticleIndexOut.local, NULL, NULL );
			//queue.enqueueReadBuffer( debugNewPos, CL_TRUE, 0, PARTICLE_COUNT * sizeof( int ), _debugNewPos );
			//queue.finish();
			//*
			
			//queue.enqueueReadBuffer( particleIndex, CL_TRUE, 0, PARTICLE_COUNT*2 * sizeof( int ), _particleIndex );
			//queue.enqueueReadBuffer( particleIndex2, CL_TRUE, 0, PARTICLE_COUNT*2 * sizeof( int ), _particleIndex2 );
			//queue.enqueueReadBuffer( workGroupOneCounts, CL_TRUE, 0, 64 * sizeof( int ), _oneCounts );
			//queue.enqueueReadBuffer( shiftBuff, CL_TRUE, 0, PARTICLE_COUNT * sizeof( int ), _shiftBuffer );
			/*
			*/
			//std::cout << "k2 " << clock.deltaTime() << std::endl;
			//k2Elapsed += clock.deltaTime();
		}
		else
		{
			//clock.frameTick();

			shiftBufferK.setArg(0, particleIndex2);
			shiftBufferK.setArg(3, mask);
			err = queue.enqueueNDRangeKernel(shiftBufferK, cl::NullRange, radixFillShiftBuffer.global, radixFillShiftBuffer.local, NULL, NULL );

			//queue.finish();
			//std::cout << "k1 " << clock.deltaTime() << std::endl;
			//k1Elapsed += clock.deltaTime();
		

			//clock.frameTick();
			//err = queue.enqueueNDRangeKernel(radixParseShiftBuffer.kernel, cl::NullRange, radixParseShiftBuffer.global, radixParseShiftBuffer.local, NULL, NULL );
			
			particleIndexOutK.setArg(0, particleIndex2);
			particleIndexOutK.setArg(1, particleIndex);
			err = queue.enqueueNDRangeKernel(particleIndexOutK, cl::NullRange, radixFillParticleIndexOut.global, radixFillParticleIndexOut.local, NULL, NULL );
			
			//queue.finish();
			//std::cout << "k2 " << clock.deltaTime() << std::endl;
			//k2Elapsed += clock.deltaTime();
		}
			
		
		mask <<= 1;
		
		//queue.enqueueReadBuffer( particleIndex, CL_TRUE, 0, PARTICLE_COUNT * 2 * sizeof( int ), _particleIndex );
		//queue.enqueueReadBuffer( workGroupOneCounts, CL_TRUE, 0, 16 * sizeof( int ), _oneCounts );
	}
	//mockK2(_shiftBuffer);
	//queue.enqueueReadBuffer( particleIndex, CL_TRUE, 0, PARTICLE_COUNT * 2 * sizeof( int ), _particleIndex );
		//std::cout << "k1: " << k1Elapsed << std::endl;
		//std::cout << "k2: " << k2Elapsed << std::endl;
		//std::cout << std::endl << std::endl;
	//queue.finish();
		//std::cout << 1.0f/clock.deltaTime() << std::endl;

	//bool isSorted = true;
	//for(int i=1; i<PARTICLE_COUNT; i++)
	//{
	//	if(_particleIndex[i*2] < _particleIndex[(i-1)*2])
	//		isSorted = false;
	//}
	//std::cout << isSorted << std::endl;
	//delete [] _particleIndex;
	//delete [] _particleIndex2;
	//delete [] _shiftBuffer;
	//delete [] _oneCounts;
	//delete [] _debugNewPos;
	//delete [] _totalOnes;
}

void Window_SPH::setupCL()
{
	//try
	{
	clInfo.initialize("Content_SPH\\Kernels\\sphFluidDemo.cl");
	makeKernels();
	makeBuffers();
	setUnchangingKernelArgs();
	//}catch(cl::Error e){
	//	std::cout << "Build log:" << std::endl << clInfo.program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(clInfo.device) << std::endl;
	}
}

void Window_SPH::makeKernels()
{
	int err = 0;
	cl::Program& program = clInfo.program;
    
	hashParticles.kernel = cl::Kernel(program, "hashParticles", &err);

	radixClearBuffers.kernel = cl::Kernel(program, "radixClearBuffers", &err);

	radixFillShiftBuffer.kernel = cl::Kernel(program, "radixFillShiftBuffer", &err);

	radixParseShiftBuffer.kernel = cl::Kernel(program, "radixParseShiftBuffer", &err);

	radixFillParticleIndexOut.kernel = cl::Kernel(program, "radixFillParticleIndexOut", &err);

	sortPostPass.kernel = cl::Kernel(program, "sortPostPass", NULL);

	indexx.kernel = cl::Kernel(program, "indexx", NULL);

	indexPostPass.kernel = cl::Kernel(program, "indexPostPass", NULL);

	findNeighbors.kernel = cl::Kernel(program, "findNeighbors", NULL);

	computeDensityPressure.kernel = cl::Kernel(program, "computeDensityPressure", NULL);

	computeAcceleration.kernel = cl::Kernel(program, "computeAcceleration", NULL);

	integrate.kernel  = cl::Kernel(program, "integrate", NULL);

	clearBuffers.kernel = cl::Kernel(program, "clearBuffers", NULL);

	cycleParticles.kernel = cl::Kernel(program, "cycleParticles", NULL);
}

void Window_SPH::setUnchangingKernelArgs()
{
	const cl::Device& device = clInfo.device;

	int gridCellCount = ((gridCellsX) * (gridCellsY)) * (gridCellsZ);
	int gridCellCountRoundedUp = ((( gridCellCount - 1 ) / 256 ) + 1 ) * 256;
	int localSize;
	size_t size;
	int err;

	cl::Kernel& clearBuffs = clearBuffers.kernel;
	clearBuffs.setArg( 0, neighborMap );
	clearBuffs.setArg( 1, workGroupOneCounts );
	
	clearBuffers.global = cl::NDRange(NEIGHBOR_COUNT * PARTICLE_COUNT * 2 / 16);
	err = clGetKernelWorkGroupInfo(clearBuffers.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	clearBuffers.local = cl::NDRange(localSize);



	cl::Kernel& hashP = hashParticles.kernel;
	hashP.setArg( 0, position );
	hashP.setArg( 1, gridCellsX );
	hashP.setArg( 2, gridCellsY );
	hashP.setArg( 3, gridCellsZ );
	hashP.setArg( 4, hashGridCellSizeInv );
	hashP.setArg( 5, xmin );
	hashP.setArg( 6, ymin );
	hashP.setArg( 7, zmin );
	hashP.setArg( 8, particleIndex );
	
	hashParticles.global = cl::NDRange(PARTICLE_COUNT);
	clGetKernelWorkGroupInfo(hashParticles.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	hashParticles.local = cl::NDRange(localSize);

	// radixSort
	radixClearBuffers.global = cl::NDRange(PARTICLE_COUNT/1024);
	radixClearBuffers.local = cl::NDRange(PARTICLE_COUNT/1024);
	radixClearBuffers.kernel.setArg(0, workGroupOneCounts);
	radixClearBuffers.kernel.setArg(1, totalOnes);

	radixFillShiftBuffer.global = cl::NDRange(PARTICLE_COUNT/8);
	clGetKernelWorkGroupInfo(radixFillShiftBuffer.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	radixFillShiftBuffer.local = cl::NDRange(localSize);
	radixFillShiftBuffer.kernel.setArg(1, shiftBuff);
	radixFillShiftBuffer.kernel.setArg(2, workGroupOneCounts);
	radixFillShiftBuffer.kernel.setArg(4, totalOnes);

	radixParseShiftBuffer.global = cl::NDRange(PARTICLE_COUNT/1024);
	radixParseShiftBuffer.local = cl::NDRange(PARTICLE_COUNT/1024);
	radixParseShiftBuffer.kernel.setArg(0, workGroupOneCounts);
	radixParseShiftBuffer.kernel.setArg(1, totalOnes);

	radixFillParticleIndexOut.global = cl::NDRange(PARTICLE_COUNT/8);
	clGetKernelWorkGroupInfo(radixFillParticleIndexOut.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	radixFillParticleIndexOut.local = cl::NDRange(localSize);
	radixFillParticleIndexOut.kernel.setArg(2, shiftBuff);
	radixFillParticleIndexOut.kernel.setArg(3, workGroupOneCounts);
	radixFillParticleIndexOut.kernel.setArg(4, totalOnes);


	// _radixSort
	// these kernels have parameters that vary every call


	cl::Kernel& sortPP = sortPostPass.kernel;
	sortPP.setArg( 0, particleIndex );
	sortPP.setArg( 1, position );
	sortPP.setArg( 2, velocity );
	sortPP.setArg( 3, sortedPosition );
	sortPP.setArg( 4, sortedVelocity );
	sortPP.setArg( 5, lifetimes );
	sortPP.setArg( 6, sortedLifetimes );
	
	sortPostPass.global = cl::NDRange(PARTICLE_COUNT);
	clGetKernelWorkGroupInfo(sortPostPass.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	sortPostPass.local = cl::NDRange(localSize);



	cl::Kernel& ixx = indexx.kernel;
	ixx.setArg( 0, particleIndex );
	ixx.setArg( 1, gridCellCount );
	ixx.setArg( 2, gridCellIndex );
	
	indexx.global = cl::NDRange(gridCellCountRoundedUp);
	clGetKernelWorkGroupInfo(indexx.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	indexx.local = cl::NDRange(localSize);



	cl::Kernel& iPP = indexPostPass.kernel;
	iPP.setArg( 0, gridCellIndex );
	iPP.setArg( 1, gridCellCount );
	iPP.setArg( 2, gridCellIndexFixedUp );
	
	indexPostPass.global = cl::NDRange(gridCellCountRoundedUp);
	clGetKernelWorkGroupInfo(indexPostPass.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	indexPostPass.local = cl::NDRange(localSize);



	cl::Kernel& fnk = findNeighbors.kernel;
	fnk.setArg( 0, gridCellIndexFixedUp );
	fnk.setArg( 1, sortedPosition );
	fnk.setArg( 2, gridCellCount );
	fnk.setArg( 3, gridCellsX );
	fnk.setArg( 4, gridCellsY );
	fnk.setArg( 5, gridCellsZ );
	fnk.setArg( 6, h );
	fnk.setArg( 7, hashGridCellSize );
	fnk.setArg( 8, hashGridCellSizeInv );
	fnk.setArg( 9, simulationScale );
	fnk.setArg( 10, xmin );
	fnk.setArg( 11, ymin );
	fnk.setArg( 12, zmin );
	fnk.setArg( 13, neighborMap );

	findNeighbors.global = cl::NDRange(PARTICLE_COUNT);
	clGetKernelWorkGroupInfo(findNeighbors.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	findNeighbors.local = cl::NDRange(localSize);



	cl::Kernel& cdp = computeDensityPressure.kernel;
	cdp.setArg( 0, neighborMap );
	cdp.setArg( 1, Wpoly6Coefficient );
	cdp.setArg( 2, h );
	cdp.setArg( 3, mass );
	cdp.setArg( 4, rho0 );
	cdp.setArg( 5, simulationScale );
	cdp.setArg( 6, stiffness );
	cdp.setArg( 7, pressure );
	cdp.setArg( 8, rho );
	cdp.setArg( 9, rhoInv );
	
	computeDensityPressure.global = cl::NDRange(PARTICLE_COUNT);
	clGetKernelWorkGroupInfo(computeDensityPressure.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	computeDensityPressure.local = cl::NDRange(localSize);



	cl::Kernel& compA = computeAcceleration.kernel;
	compA.setArg( 0, neighborMap );
	compA.setArg( 1, pressure );
	compA.setArg( 2, rho );
	compA.setArg( 3, rhoInv );
	compA.setArg( 4, sortedPosition );
	compA.setArg( 5, sortedVelocity );
	compA.setArg( 6, CFLLimit );
	compA.setArg( 7, del2WviscosityCoefficient );
	compA.setArg( 8, gradWspikyCoefficient );
	compA.setArg( 9, h );
	compA.setArg( 10, mass );
	compA.setArg( 11, mu );
	compA.setArg( 12, simulationScale );
	compA.setArg( 13, acceleration );

	computeAcceleration.global = cl::NDRange(PARTICLE_COUNT);
	clGetKernelWorkGroupInfo(computeAcceleration.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	computeAcceleration.local = cl::NDRange(localSize);



	cl::Kernel& integrateK = integrate.kernel;
	integrateK.setArg( 0, acceleration );
	integrateK.setArg( 1, sortedPosition );
	integrateK.setArg( 2, sortedVelocity );
	integrateK.setArg( 3, gravity_x );
	integrateK.setArg( 4, gravity_y );
	integrateK.setArg( 5, gravity_z );
	integrateK.setArg( 6, simulationScaleInv );
	integrateK.setArg( 7, timeStep );
	integrateK.setArg( 8, xmin );
	integrateK.setArg( 9, xmax );
	integrateK.setArg( 10, ymin );
	integrateK.setArg( 11, ymax );
	integrateK.setArg( 12, zmin );
	integrateK.setArg( 13, zmax );
	integrateK.setArg( 14, damping );
	integrateK.setArg( 15, position );
	integrateK.setArg( 16, velocity );

	integrate.global = cl::NDRange(PARTICLE_COUNT);
	clGetKernelWorkGroupInfo(integrate.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	integrate.local = cl::NDRange(localSize);

	cl::Kernel& cycleK = cycleParticles.kernel;
	cycleK.setArg(0, position);
	cycleK.setArg(1, velocity);
	//cycleK.setArg(2, gridCellsX/2 + gridCellsY/2 * gridCellsX + gridCellsZ/2 * gridCellsX * gridCellsY);
	cycleK.setArg(2, 256);
	cycleK.setArg(3, 0);
	cycleK.setArg(4, gridCellsX);
	cycleK.setArg(5, gridCellsY);
	cycleK.setArg(6, gridCellsZ);
	cycleK.setArg(7, hashGridCellSize);
	cycleK.setArg(8, sourceVelocity);
	cycleK.setArg(9, endSize); //endSize
	cycleK.setArg(10, 40.0f); // endMinX
	cycleK.setArg(11, 45.0f); // endMinZ
	cycleK.setArg(12, 1.0f/60.0f); // timeStep
	cycleK.setArg(13, lifetimes); // lifetimes
	cycleK.setArg(14, sortedLifetimes); // sortedLifetimes
	
	cycleParticles.global = cl::NDRange(PARTICLE_COUNT);
	clGetKernelWorkGroupInfo(cycleParticles.kernel(), device(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &localSize, &size);
	cycleParticles.local = cl::NDRange(localSize);
}

void Window_SPH::makeBuffers()
{
	cl::CommandQueue& queue = clInfo.queue;
	cl::Context& gpuContext = clInfo.context;
	int err;

	glm::vec4* position_ = new glm::vec4[PARTICLE_COUNT];
	glm::vec4* velocity_ = new glm::vec4[PARTICLE_COUNT];
	glm::vec4* acceleration_ = new glm::vec4[PARTICLE_COUNT];

	uint* particleIndex_ = new uint[PARTICLE_COUNT*2];
	uint* particleIndex2_ = new uint[PARTICLE_COUNT*2];
	uint* shiftBuff_ = new uint[PARTICLE_COUNT];
	uint* workGroupOneCounts_ = new uint[PARTICLE_COUNT/1024];

	glm::vec4* sortedPosition_ = new glm::vec4[PARTICLE_COUNT];
	glm::vec4* sortedVelocity_ = new glm::vec4[PARTICLE_COUNT];

	uint* gridCellIndex_ = new uint[gridCellCount+1];
	uint* gridCellIndexFixedUp_ = new uint[gridCellCount+1];

	float* lifetimes_ = new float[PARTICLE_COUNT];
	
	int ppp = xLayers * zLayers;
	for(int i=0; i<PARTICLE_COUNT; i++)
	{
		acceleration_[i] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		int particleThisPlane = i%ppp;
		position_[i].x = particleThisPlane%xLayers * xDim;
		position_[i].y = i/(PARTICLE_COUNT/8) * 1.3f;
		position_[i].z = particleThisPlane/xLayers * zDim;
		lifetimes_[i] = 0.0f;
	}
	
	acceleration = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( PARTICLE_COUNT * sizeof( float ) * 4 ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer acceleration creation failed" );
	}
	gridCellIndex = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( ( gridCellCount + 1 ) * sizeof( unsigned int ) * 1 ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer gridCellIndex creation failed" );
	}
	gridCellIndexFixedUp = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( ( gridCellCount + 1 ) * sizeof( unsigned int ) * 1 ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer gridCellIndexFixedUp creation failed" );
	}
	neighborMap = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( NK * sizeof( float ) * 2 ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer neighborMap creation failed" );
	}


	particleIndex = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( PARTICLE_COUNT * sizeof( unsigned int ) * 2 ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer particleIndex creation failed" );
	}
	particleIndex2 = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( PARTICLE_COUNT * sizeof( unsigned int ) * 2 ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer particleIndex2 creation failed" );
	}
	shiftBuff = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( PARTICLE_COUNT * sizeof( unsigned int ) ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer shiftBuff creation failed" );
	}
	workGroupOneCounts = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( PARTICLE_COUNT/1024 * sizeof( unsigned int ) ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer workGroupOneCounts creation failed" );
	}
	totalOnes = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( 2 * sizeof( unsigned int ) ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer totalOnes creation failed" );
	}
	pressure = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( PARTICLE_COUNT * sizeof( float ) * 1 ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer pressure creation failed" );
	}
	rho = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( PARTICLE_COUNT * sizeof( float ) * 1 ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer rho creation failed" );
	}
	rhoInv = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( PARTICLE_COUNT * sizeof( float ) * 1 ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer rhoInv creation failed" );
	}
	sortedPosition = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( PARTICLE_COUNT * sizeof( float ) * 4 ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer sortedPosition creation failed" );
	}
	sortedVelocity = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( PARTICLE_COUNT * sizeof( float ) * 4 ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer sortedVelocity creation failed" );
	}
	velocity = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( PARTICLE_COUNT * sizeof( float ) * 4 ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer velocity creation failed" );
	}

	sortedLifetimes = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, PARTICLE_COUNT * sizeof(float), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer sortedLifetimes creation failed" );
	}

	debugNewPos = cl::Buffer( gpuContext, CL_MEM_READ_WRITE, ( PARTICLE_COUNT * sizeof( uint ) ), NULL, &err );
	if( err != CL_SUCCESS ){
		throw std::runtime_error( "buffer debugNewPos creation failed" );
	}

	queue.enqueueWriteBuffer(velocity, true, 0, sizeof(glm::vec4)*PARTICLE_COUNT, velocity_, NULL, NULL);
	queue.enqueueWriteBuffer(acceleration, true, 0, sizeof(glm::vec4)*PARTICLE_COUNT, acceleration_, NULL, NULL);
	//queue.enqueueWriteBuffer(gridCellIndexFixedUp, true, 0, sizeof(uint)*PARTICLE_COUNT, gridCellIndexFixedUp_, NULL, NULL);
	queue.enqueueWriteBuffer(particleIndex, true, 0, sizeof(uint)*2*PARTICLE_COUNT, particleIndex_, NULL, NULL);
	queue.enqueueWriteBuffer(particleIndex2, true, 0, sizeof(uint)*2*PARTICLE_COUNT, particleIndex2_, NULL, NULL);
	queue.enqueueWriteBuffer(shiftBuff, true, 0, sizeof(uint)*PARTICLE_COUNT, shiftBuff_, NULL, NULL);
	queue.enqueueWriteBuffer(workGroupOneCounts, true, 0, sizeof(uint)*PARTICLE_COUNT/1024, workGroupOneCounts_, NULL, NULL);

	queue.enqueueWriteBuffer(sortedPosition, true, 0, sizeof(glm::vec4)*PARTICLE_COUNT, sortedPosition_, NULL, NULL);
	queue.enqueueWriteBuffer(sortedVelocity, true, 0, sizeof(glm::vec4)*PARTICLE_COUNT, sortedVelocity_, NULL, NULL);
	queue.enqueueWriteBuffer(sortedLifetimes, true, 0, sizeof(float)*PARTICLE_COUNT, lifetimes_, NULL, NULL);

	queue.enqueueWriteBuffer(gridCellIndex, true, 0, sizeof(uint)*gridCellCount+1, gridCellIndex_, NULL, NULL);
	
	uint posID = geo->addAttribBuffer(4, GL_FLOAT, sizeof(glm::vec4), position_, sizeof(glm::vec4)*PARTICLE_COUNT, GL_DYNAMIC_DRAW);
	position() = clCreateFromGLBuffer(gpuContext(), CL_MEM_READ_WRITE, posID, NULL);

	uint lifetimesID = geo->addAttribBuffer(1, GL_FLOAT, sizeof(float), lifetimes_, sizeof(float) * PARTICLE_COUNT);
	lifetimes() = clCreateFromGLBuffer(gpuContext(), CL_MEM_READ_WRITE, lifetimesID, NULL);

	delete [] position_;
	delete [] velocity_;
	delete [] acceleration_;
	delete [] lifetimes_;
	
	delete [] particleIndex_;
	delete [] particleIndex2_;
	delete [] shiftBuff_;
	delete [] workGroupOneCounts_;
	
	delete [] sortedPosition_;
	delete [] sortedVelocity_;

	delete [] gridCellIndex_;
	delete [] gridCellIndexFixedUp_;
}

