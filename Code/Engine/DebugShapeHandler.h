#ifndef DEBUGSHAPEHANDLER_H
#define DEBUGSHAPEHANDLER_H
#include "typedefs_Engine.h"
#ifdef ENGINE_DEBUG
#include "Renderer.h"

namespace Engine
{
	class ENGINE_SHARED DebugShapeHandler
	{

	#pragma region Instance Variabes
		uint vAttrib_sizes[2];
		uint vAttrib_offsets[2];
		float timeSinceLastShapeAdded;
		uint currentDebugRenderablesIndex;

		float lifetimes[RENDERABLES_MAX_CAPACITY];
		Renderer* targetRenderer;
	#pragma endregion

	#pragma region Initializer
	public:
		void initialize(Renderer* targetRenderer);
	#pragma endregion

	#pragma region Internal Struct
		struct DebugRenderableContainer
		{
			Renderer::Renderable* targetRenderable;
			float lifetime;
			float lived;
			void initialize(Renderer::Renderable* targetRenderable, float lifetime);
		} debugRenderables[RENDERABLES_MAX_CAPACITY];
	#pragma endregion
			
	#pragma region Timing and Frame-based Functions
	public:	
		void tickAllLifetimes(float deltaTime);
		bool enoughTimeElapsedSinceLastAdd();
	#pragma endregion
		
	#pragma region Lazy-loading Functions
	private:
		uint makeArrow();
		uint makeSphere();
		uint makeCube();
		uint makeLine();
		uint makePoint();
		uint makeAxes();
		uint makeBoundingBoxForGeometry(uint geometryID, uint vAttribID_position);
	#pragma endregion
		
	#pragma region Public Functions
	public:
		Renderer::Renderable* addArrow(uint shaderProgram, float lifetime, bool useDepthTesting);
		Renderer::Renderable* addSphere(uint shaderProgram, float lifetime, bool useDepthTesting);
		Renderer::Renderable* addCube(uint shaderProgram, float lifetime, bool useDepthTesting);
		Renderer::Renderable* addLine(uint shaderProgram, float lifetime, bool useDepthTesting);
		Renderer::Renderable* addPoint(uint shaderProgram, float lifetime, bool useDepthTesting);
		Renderer::Renderable* addBoundingBox(uint geometryID, uint shaderProgram, float lifetime, bool useDepthTesting);
	#pragma endregion
	
	};
}

#endif
#endif