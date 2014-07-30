#include "Window_MD.h"
#include <Renderer.h>
#include <FbxLoader.h>
#include <LevelData.h>
#include <Qt\qdir.h>
#include <DebugMenuManager.h>
#include "RippleEffect.h"
#include "WaveEffect.h"
#include <glm\gtx\transform.hpp>
#include <ShapeGenerator.h>
#include <iostream>
#include <fstream>


Window_MD::Window_MD()
{
	crosshairTransform = NULL;
}

static float clamp(float val, float min, float max)
{
	return (val>min) ? ((val<max) ? val: max) : min;
}

void Window_MD::initialize()
{
	renderer->camera.position = glm::vec3(-20.3176f, 10.7037f, 0.531963f);
	renderer->camera.facing = glm::normalize(glm::vec3(0.759272f, -0.650775f, 0.0f));
	renderer->camera.MovementMultiplier = 1.5f;
	oldestRippleIndex = 0;
	framesSinceLastKey = 0;

	Engine::LevelData ld;
	ld.read_native("Content_MD\\MeshDeformation.bin");
	Neumont::ShapeData* mesh = ld.shapes;

	uint sizesMD[2];
		sizesMD[0] = 3;
		sizesMD[1] = 2;

	uint positionsMD[2];
		positionsMD[0] = Neumont::Vertex::POSITION_OFFSET;
		positionsMD[1] = Neumont::Vertex::UV_OFFSET;

	uint sizesPT[2];
		sizesPT[0] = 3;
		sizesPT[1] = 3;

	uint positionsPT[2];
		positionsPT[0] = Neumont::Vertex::POSITION_OFFSET;
		positionsPT[1] = Neumont::Vertex::POSITION_OFFSET;
	

	uint geo = renderer->addGeometry(mesh->vertexBufferSize(), mesh->indexBufferSize(), mesh->numIndices, mesh->verts, mesh->indices,
		Neumont::Vertex::STRIDE, 2, sizesMD, positionsMD, Engine::Renderer::LINES);
	
	uint crosshairGeoID = renderer->addGeometry(mesh[1].vertexBufferSize(), mesh[1].indexBufferSize(), mesh[1].numIndices, mesh[1].verts, mesh[1].indices, Neumont::Vertex::STRIDE, 
		2, sizesPT, positionsPT, Engine::Renderer::TRIANGLES);

	uint basicShaderID = renderer->addProgram("Content_MD\\PassThrough.vsh", "Content_MD\\PassThrough.fsh");
	//uint meshShaderID = renderer->addProgram("Content_MD\\SurfaceMesh.vsh", "Content_MD\\SurfaceMesh.fsh", "Content_MD\\SurfaceMesh.gsh");
	uint meshShaderID = renderer->addProgram("Content_MD\\SurfaceMesh.vsh", "Content_MD\\SurfaceMesh.fsh");
	
	Engine::Renderer::Renderable* r = renderer->addRenderable(geo, meshShaderID, true);
	r->addUniform(Engine::Renderer::GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE, "transform", 64, &glm::mat4());
	meshFacing = r->addUniform(Engine::Renderer::GLMVEC3, "camFacing", 12, &glm::vec3(1.0f, 0.0f, 0.0f));
	

	Engine::Renderer::Renderable* crosshairR = renderer->addRenderable(crosshairGeoID, basicShaderID, false);
	crosshairTransform = crosshairR->addUniform(Engine::Renderer::GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE, "transform", 64, &glm::mat4());

	RippleEffect rippleEffects[50];
	
	rippleEffects[0].active = 1;
	rippleEffects[0].origin = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	rippleEffects[0].rippleDistance = 0.0f;

	rippleEffects[1].active = 1;
	rippleEffects[1].origin = glm::vec4(4.0f, 0.0f, -4.0f, 0.0f);
	rippleEffects[1].rippleDistance = 0.0f;

	rippleEffects[2].active = 1;
	rippleEffects[2].origin = glm::vec4(-4.0f, 0.0f, 4.0f, 0.0f);
	rippleEffects[2].rippleDistance = 0.0f;

	rippleEffects[3].active = 1;
	rippleEffects[3].origin = glm::vec4(-4.0f, 0.0f, -4.0f, 0.0f);
	rippleEffects[3].rippleDistance = 0.0f;

	for(uint i=0; i<50; i++)
	{
		rippleEffects[i].active = 0;
	}

	std::cout << sizeof(RippleEffect) << std::endl;

	WaveEffect waveEffects[10];

	//waveEffects[0].waveDir = glm::vec3(0.0f, 0.0f, -1.0f);
	//waveEffects[0].waveDistance = 0.0f;
	//waveEffects[0].origin = glm::vec3(10.0f, 0.0f, 10.0f);
	//waveEffects[0].active = true;

	//waveEffects[1].waveDir = glm::vec3(0.0f, 0.0f, 1.0f);
	//waveEffects[1].waveDistance = 0.0f;
	//waveEffects[1].origin = glm::vec3(-10.0f, 0.0f, -10.0f);
	//waveEffects[1].active = true;

	for(uint i=0; i<10; i++)
	{
		waveEffects[i].active = false;
	}

	ripples = r->addUniformBlock("RippleEffects", sizeof(RippleEffect), 50, rippleEffects);
	waves = r->addUniformBlock("WaveEffects", sizeof(WaveEffect), 10, waveEffects);
}

void Window_MD::inheritedUpdate()
{
	if(initialized)
	{
		glm::mat4 xform = glm::translate(renderer->camera.position + renderer->camera.facing * 0.1f) * glm::scale(0.0005f, 0.0005f, 0.0005f);
		crosshairTransform->updateInfo(64, &xform);
		meshFacing->updateInfo(12, &(renderer->camera.facing));

		if(GetAsyncKeyState(Qt::Key::Key_Z))
		{
			float x = 0.0f;
			int y = 0;
			for(uint i=0; i<ripples->numObjects; i++)
			{
				ripples->updateMember(i, sizeof(glm::vec4), sizeof(float), &x);
				ripples->updateMember(i, sizeof(glm::vec4) + sizeof(float), sizeof(int), &y);
			}
			for(uint i=0; i<waves->numObjects; i++)
			{
				waves->updateMember(i, sizeof(glm::vec3), sizeof(float), &x);
				waves->updateMember(i, sizeof(glm::vec3) * 2 + sizeof(float), sizeof(int), &y);
			}
		}
		if(GetAsyncKeyState(Qt::Key::Key_X))
		{
			float x;
			for(uint i=0; i<ripples->numObjects; i++)
			{
				ripples->getMemberCopy(i, sizeof(glm::vec4), sizeof(float), &x);
				x -= 0.1f;
				ripples->updateMember(i, sizeof(glm::vec4), sizeof(float), &x);
			}
			for(uint i=0; i<waves->numObjects; i++)
			{
				waves->getMemberCopy(i, sizeof(glm::vec3), sizeof(float), &x);
				x -= 0.1f;
				waves->updateMember(i, sizeof(glm::vec3), sizeof(float), &x);
			}
		}
		/*if(GetAsyncKeyState(Qt::Key::Key_C))
		{
			float x;
			for(uint i=0; i<ripples->numObjects; i++)
			{
				ripples->getMemberCopy(i, sizeof(glm::vec3), sizeof(float), &x);
				x += 0.1f;
				ripples->updateMember(i, sizeof(glm::vec3), sizeof(float), &x);
			}
			for(uint i=0; i<waves->numObjects; i++)
			{
				waves->getMemberCopy(i, sizeof(glm::vec3), sizeof(float), &x);
				x += 0.1f;
				waves->updateMember(i, sizeof(glm::vec3), sizeof(float), &x);
			}
		}*/

		float distance;
		for(uint i=0; i<ripples->numObjects; i++)
		{
			ripples->getMemberCopy(i, sizeof(glm::vec4), sizeof(float), &distance);
			distance += 0.1f;
			ripples->updateMember(i, sizeof(glm::vec4), sizeof(float), &distance);
		}
		for(uint i=0; i<waves->numObjects; i++)
		{
			waves->getMemberCopy(i, sizeof(glm::vec3), sizeof(float), &distance);
			distance += 0.1f;
			waves->updateMember(i, sizeof(glm::vec3), sizeof(float), &distance);
		}
		//std::cout << 1.0 / clock.deltaTime() << std::endl;

		if(GetAsyncKeyState(Qt::Key::Key_N) && framesSinceLastKey > 4)
		{
			framesSinceLastKey = 0;

			float numFacingsToPlane = renderer->camera.position.y / -renderer->camera.facing.y;
			if (numFacingsToPlane > 0)
			{
				RippleEffect ripple;
				ripple.active = 1;
				ripple.origin = glm::vec4(renderer->camera.position + renderer->camera.facing * numFacingsToPlane, 0.0f);
				ripple.rippleDistance = 0.0f;

				ripples->updateObject(oldestRippleIndex++, &ripple);
				if(oldestRippleIndex >= ripples->numObjects)
				{
					oldestRippleIndex = 0;
				}
			}
		}

		if(GetAsyncKeyState(Qt::Key::Key_B))
		{
			std::cout << renderer->camera.facing.x << ", " << renderer->camera.facing.y << ", " <<  renderer->camera.facing.z << std::endl;
			renderer->camera.position.y = 0.0f;
		}

		if(GetAsyncKeyState(Qt::Key::Key_M) && framesSinceLastKey > 8)
		{
			framesSinceLastKey = 0;

			WaveEffect wave;
			glm::vec3 x(1.0f, 0.0f, 0.0f);
			glm::vec3 z(0.0f, 0.0f, 1.0f);
			//glm::vec3& pos = renderer->camera.position;
			
			float numFacingsToPlane = renderer->camera.position.y / -renderer->camera.facing.y;
			if(numFacingsToPlane > 0)
			{
				glm::vec3 pos = renderer->camera.position + renderer->camera.facing * numFacingsToPlane;
				wave.origin = glm::vec3(clamp(glm::dot(pos, x), -10.0f, 10.0f), 0.0f, clamp(glm::dot(pos, z), -10.0f, 10.0f));
				wave.waveDistance = 0.0f;
				wave.waveDir = glm::normalize(glm::vec3(renderer->camera.facing.x, 0.0f, renderer->camera.facing.z));
				wave.active = true;

				waves->updateObject(oldestWaveIndex++, &wave);
				if(oldestWaveIndex >= waves->numObjects)
				{
					oldestWaveIndex = 0;
				}
			}
		}
		framesSinceLastKey++;
	}
}