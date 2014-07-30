#include "typedefs_Engine.h"
#ifdef ENGINE_DEBUG
#include "DebugShapeHandler.h"
#include "ShapeGenerator.h"
#include <Qt\qdebug.h>

namespace Engine
{
#pragma region Initializer
	void DebugShapeHandler::initialize(Renderer* targetRendererIn)
	{
		targetRenderer = targetRendererIn;
		timeSinceLastShapeAdded = 0.0f;
		currentDebugRenderablesIndex = 0;
		vAttrib_sizes[0] = 3; // position
		vAttrib_sizes[1] = 4; // color
		
		vAttrib_offsets[0] = Neumont::Vertex::POSITION_OFFSET;
		vAttrib_offsets[1] = Neumont::Vertex::COLOR_OFFSET;
	}
#pragma endregion

#pragma region Internal Struct Initializer
	void DebugShapeHandler::DebugRenderableContainer::initialize(Renderer::Renderable* targetRenderableIn, float lifetimeIn)
	{
		targetRenderable = targetRenderableIn;
		lifetime = lifetimeIn;
		lived = 0.0f;
	}
#pragma endregion 

#pragma region Timing and Frame-based Functions
	void DebugShapeHandler::tickAllLifetimes(float deltaTime)
	{
		timeSinceLastShapeAdded += deltaTime;
		for(unsigned int i=0;i<currentDebugRenderablesIndex;i++)
		{
			if(debugRenderables[i].lifetime > debugRenderables[i].lived)
			{
				debugRenderables[i].lived += deltaTime;
				if(debugRenderables[i].lifetime < debugRenderables[i].lived)
					debugRenderables[i].targetRenderable->visible = false;
			}
		}
	}

	bool DebugShapeHandler::enoughTimeElapsedSinceLastAdd()
	{
		return timeSinceLastShapeAdded >= 1.5f;
	}
#pragma endregion

#pragma region Lazy-loading Functions
	uint DebugShapeHandler::makeArrow()
	{
		static bool notMade = true;
		static uint arrowID = 0;
		if(notMade)
		{
			Neumont::ShapeData shape(Neumont::ShapeGenerator::makeArrow());
			arrowID = targetRenderer->addGeometry(shape.vertexBufferSize(), shape.indexBufferSize(),
				shape.numIndices, shape.verts, shape.indices, Neumont::Vertex::STRIDE,
				2, vAttrib_sizes, vAttrib_offsets, Renderer::DrawStyle::TRIANGLES);

			qDebug() << "made arrow";
			notMade = false;
			shape.cleanUp();
		}
		else
		{
			qDebug() << "already made arrow";
		}

		return arrowID;
	}

	uint DebugShapeHandler::makeSphere()
	{
		static bool notMade = true;
		static uint sphereID=0;
		if(notMade)
		{
			Neumont::ShapeData shape(Neumont::ShapeGenerator::makeSphere(10));
			sphereID = targetRenderer->addGeometry(shape.vertexBufferSize(), shape.indexBufferSize(),
				shape.numIndices, shape.verts, shape.indices, Neumont::Vertex::STRIDE,
				2, vAttrib_sizes, vAttrib_offsets, Renderer::DrawStyle::TRIANGLES);
			qDebug() << "made sphere";
			notMade = false;
			shape.cleanUp();
		}
		else
		{
			qDebug() << "already made sphere";
		}
		return sphereID;
	}

	uint DebugShapeHandler::makeCube()
	{
		static bool notMade = true;
		static uint cubeID = 0;
		if(notMade)
		{
			Neumont::ShapeData shape(Neumont::ShapeGenerator::makeCube());
			cubeID = targetRenderer->addGeometry(shape.vertexBufferSize(), shape.indexBufferSize(),
				shape.numIndices, shape.verts, shape.indices, Neumont::Vertex::STRIDE,
				2, vAttrib_sizes, vAttrib_offsets, Renderer::DrawStyle::TRIANGLES);
			qDebug() << "made cube";
			notMade = false;
			shape.cleanUp();
		}
		else
		{
			qDebug() << "already made cube";
		}
		return cubeID;
	}

	uint DebugShapeHandler::makeLine()
	{
		static bool notMade = true;
		static uint lineID=0;
		if(notMade)
		{
			Neumont::ShapeData shape(Neumont::ShapeGenerator::makeLine());
			lineID = targetRenderer->addGeometry(shape.vertexBufferSize(), shape.indexBufferSize(),
				shape.numIndices, shape.verts, shape.indices, Neumont::Vertex::STRIDE,
				2, vAttrib_sizes, vAttrib_offsets, Renderer::DrawStyle::LINES);
			qDebug() << "made line";
			notMade = false;
			shape.cleanUp();
		}
		else
		{
			qDebug() << "already made line";
		}
		return lineID;
	}

	uint DebugShapeHandler::makePoint()
	{
		static bool notMade = true;
		static uint pointID = 0;
		if(notMade)
		{
				Neumont::Vertex* verts = new Neumont::Vertex[4];
				verts[0].position = glm::vec3(0.0f, 0.0f, 0.0f);
				verts[0].color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				verts[1].position = glm::vec3(1.0f, 0.0f, 0.0f);
				verts[1].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
				verts[2].position = glm::vec3(0.0f, 1.0f, 0.0f);
				verts[2].color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
				verts[3].position = glm::vec3(0.0f, 0.0f, 1.0f);
				verts[3].color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

			ushort* indices = new ushort[6];
				indices[0] = 0;
				indices[1] = 1;
				indices[2] = 0;
				indices[3] = 2;
				indices[4] = 0;
				indices[5] = 3;
				
				pointID = targetRenderer->addGeometry(sizeof(Neumont::Vertex)*4, sizeof(uint)*6, 6, verts, indices, Neumont::Vertex::STRIDE,
				2, vAttrib_sizes, vAttrib_offsets, Renderer::DrawStyle::LINES);
			qDebug() << "made point";
			notMade = false;

			delete [] verts;
			delete [] indices;
		}
		else
		{
			qDebug() << "already made point";
		}
		return pointID;
	}

	uint DebugShapeHandler::makeBoundingBoxForGeometry(uint geometryID, uint vAttribID_position)
	{
		
		Renderer::GeometryInfo* geometry = targetRenderer->debug_getGeometryInfoAtIndex(geometryID);
		
		ushort* indicesHolder = new ushort[geometry->numIndices];
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->iBuffer);
		glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, geometry->iOffset, geometry->numIndices * sizeof(ushort), indicesHolder);

		ushort maxIndex = 0;
		for(unsigned int i=0; i<geometry->numIndices; i++)
		{
			if(maxIndex < indicesHolder[i])
				maxIndex = indicesHolder[i];
		}

		int sizeToPullfromVertBuffer = geometry->stride * maxIndex;
		char* verticesHolder = new char[sizeToPullfromVertBuffer];

		glBindBuffer(GL_ARRAY_BUFFER, geometry->vBuffer);
		glGetBufferSubData(GL_ARRAY_BUFFER, geometry->vOffset, sizeToPullfromVertBuffer, verticesHolder);

		float maxX = 0.0f;
		float minX = 0.0f;
		float maxY = 0.0f;
		float minY = 0.0f;
		float maxZ = 0.0f;
		float minZ = 0.0f;
		
		
		for(int i=0; i<maxIndex; i++)
		{
			float* position = reinterpret_cast<float*>( &(verticesHolder[i*geometry->stride + geometry->vAttribs[vAttribID_position].offset]) );
			
			float x = position[0];
			float y = position[1];
			float z = position[2];
			

			if( x > maxX && x<10000.0f)
				maxX = x;
			if( x < minX && x>-10000.0f)
				minX = x;
			
			if( y > maxY && y<10000.0f)
				maxY = y;
			if( y < minY && y>-10000.0f)
				minY = y;
		
			if( z > maxZ && z<10000.0f)
				maxZ = z;
			if( z < minZ && z>-10000.0f)
				minZ = z;
		}


		float* verts = new float[24];
		
		verts[0]=minX;  verts[1]=minY;  verts[2]=minZ;
		verts[3]=minX;  verts[4]=minY;  verts[5]=maxZ;
		verts[6]=minX;  verts[7]=maxY;  verts[8]=minZ;
		verts[9]=minX;  verts[10]=maxY; verts[11]=maxZ;
		verts[12]=maxX; verts[13]=minY; verts[14]=minZ;
		verts[15]=maxX; verts[16]=minY; verts[17]=maxZ;
		verts[18]=maxX; verts[19]=maxY; verts[20]=minZ;
		verts[21]=maxX; verts[22]=maxY; verts[23]=maxZ;

		ushort* indices = new ushort[24];
		
		indices[0]=0; indices[1]=1;
		indices[2]=0; indices[3]=2;
		indices[4]=0; indices[5]=4;
		
		indices[6]=1; indices[7]=3;
		indices[8]=1; indices[9]=5;
		
		indices[10]=2; indices[11]=3;
		indices[12]=2; indices[13]=6;
		
		indices[14]=3; indices[15]=7;
		
		indices[16]=4; indices[17]=5;
		indices[18]=4; indices[19]=6;
		
		indices[20]=5; indices[21]=7;
		
		indices[22]=6; indices[23]=7;

		uint size = 3;
		uint offset = 0;

		
		uint returnValue = targetRenderer->addGeometry(sizeof(float)*24, sizeof(ushort)*24, 24, verts, indices, 12, 1, &size, &offset, Renderer::DrawStyle::LINES);

		delete [] indicesHolder;
		delete [] verticesHolder;
		delete [] verts;
		delete [] indices;

		return returnValue;
	}
#pragma endregion

#pragma region Public Functions
	Renderer::Renderable* DebugShapeHandler::addArrow(uint shaderProgram, float lifetime, bool useDepthTesting)
	{
		timeSinceLastShapeAdded = 0.0f;
		debugRenderables[currentDebugRenderablesIndex].initialize(targetRenderer->addRenderable(makeArrow(), shaderProgram, useDepthTesting), lifetime);
		return debugRenderables[currentDebugRenderablesIndex++].targetRenderable;
	}

	Renderer::Renderable* DebugShapeHandler::addSphere(uint shaderProgram, float lifetime, bool useDepthTesting)
	{
		timeSinceLastShapeAdded = 0.0f;
		debugRenderables[currentDebugRenderablesIndex].initialize(targetRenderer->addRenderable(makeSphere(), shaderProgram, useDepthTesting), lifetime);
		return debugRenderables[currentDebugRenderablesIndex++].targetRenderable;
	}

	Renderer::Renderable* DebugShapeHandler::addCube(uint shaderProgram, float lifetime, bool useDepthTesting)
	{
		timeSinceLastShapeAdded = 0.0f;
		debugRenderables[currentDebugRenderablesIndex].initialize(targetRenderer->addRenderable(makeCube(), shaderProgram, useDepthTesting), lifetime);
		return debugRenderables[currentDebugRenderablesIndex++].targetRenderable;
	}

	Renderer::Renderable* DebugShapeHandler::addLine(uint shaderProgram, float lifetime, bool useDepthTesting)
	{
		timeSinceLastShapeAdded = 0.0f;
		debugRenderables[currentDebugRenderablesIndex].initialize(targetRenderer->addRenderable(makeLine(), shaderProgram, useDepthTesting), lifetime);
		return debugRenderables[currentDebugRenderablesIndex++].targetRenderable;
	}

	Renderer::Renderable* DebugShapeHandler::addPoint(uint shaderProgram, float lifetime, bool useDepthTesting)
	{
		timeSinceLastShapeAdded = 0.0f;
		debugRenderables[currentDebugRenderablesIndex].initialize(targetRenderer->addRenderable(makePoint(), shaderProgram, useDepthTesting), lifetime);
		return debugRenderables[currentDebugRenderablesIndex++].targetRenderable;
	}

	Renderer::Renderable* DebugShapeHandler::addBoundingBox(uint geometryID, uint shaderProgram, float lifetime, bool useDepthTesting)
	{
		timeSinceLastShapeAdded = 0.0f;
		//TODO: change the magic number 0 in the below line (it represents the index of the vertex attrib that holds position and is used in finding the vertex positions from the vertex buffer)
		debugRenderables[currentDebugRenderablesIndex].initialize(targetRenderer->addRenderable(makeBoundingBoxForGeometry(geometryID, 0), shaderProgram, useDepthTesting), lifetime);
		return debugRenderables[currentDebugRenderablesIndex++].targetRenderable;
	}
#pragma endregion
}
#endif