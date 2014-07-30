#include "Renderer.h"
#include <Qt\qevent.h>
#include <Qt\qtextstream.h>
#include <Qt\qdebug.h>
#include <Qt\qfile.h>
#include "DebugShapeHandler.h"
#include <glm\gtx\transform.hpp>
#include <iostream>
#include <fstream>
#include <cstring>

namespace Engine
{

	Renderer::Renderer()
	{
		nextAvailableUniformBlockBindingPoint = 1;
		paused = false;
		nextVertexOffset[0] = 0;
		nextIndexOffset[0] = 0;
		nextUniformOffset[0] = 0;
		
		currentGeometriesIndex = 0;
		currentBufferedGeometriesIndex = 0;
		currentShaderProgramsIndex = 0;
		currentRenderablesIndex = 0;
		currentVertexBufferIndex = 0;
		currentIndexBufferIndex = 0;
		currentUniformBufferIndex = 0;
		nextTextureIndex = 0;
	}

	#pragma region Internal Struct Initializers and Functions

	#pragma region UniformInfo
	
	void Renderer::UniformInfo::initialize(UniformFormat formatIn, const char* nameIn, uint dataSize, void* dataIn)
	{
		assert (UNIFORMNAME_MAX_CAPACITY >= strlen(nameIn));
		memset(name, '\0', UNIFORMNAME_MAX_CAPACITY);
		memcpy(name, nameIn, strlen(nameIn));

		format = formatIn;
		memcpy(data, dataIn, dataSize);
	}

	void Renderer::UniformInfo::updateInfo( uint dataSize, void* dataIn)
	{
		memcpy(data, dataIn, dataSize);
	}
	
#pragma endregion

	#pragma region GeometryInfo
	
	void Renderer::GeometryInfo::initialize(uint vBufferIn, uint iBufferIn, uint vOffsetIn, uint iOffsetIn, uint numIndicesIn, uint strideIn, uint numVAttribsIn, uint* sizes, uint* offsets, DrawStyle drawStyleIn)
	{
		vBuffer = vBufferIn;
		iBuffer = iBufferIn;
		vOffset = vOffsetIn;
		iOffset = iOffsetIn;

		numIndices = numIndicesIn;
		stride = strideIn;
		numVAttribs = numVAttribsIn;

		drawStyle = drawStyleIn;

		for( uint i=0; i<numVAttribs; i++)
		{
			vAttribs[i].size = sizes[i];
			vAttribs[i].offset = offsets[i];
		}
	}

	#pragma endregion

	#pragma region BufferedGeometryInfo

	Renderer::BufferedGeometryInfo::BufferedGeometryInfo()
	{
		numAttribs = 0;
		numUniforms = 0;
	}

	uint Renderer::BufferedGeometryInfo::addAttribBuffer(uint numGlTypes, uint glType, uint stride, void* data, uint dataSize, uint usageHint)
	{
		AttribBufferInfo& attrib = attribs[numAttribs];
		glGenBuffers(1, &attrib.buffer);
		glBindBuffer(GL_ARRAY_BUFFER, attrib.buffer);
		glBufferData(GL_ARRAY_BUFFER, dataSize, data, usageHint);

		attrib.size = numGlTypes;
		attrib.glType = glType;
		attrib.stride = stride;

		numAttribs++;
		return attrib.buffer;
	}

	uint Renderer::BufferedGeometryInfo::addAttribBuffer(uint numGlTypes, uint glType, uint stride, void* data, uint dataSize)
	{
		return addAttribBuffer(numGlTypes, glType, stride, data, dataSize, GL_STREAM_DRAW);
	}

	Renderer::UniformInfo* Renderer::BufferedGeometryInfo::addUniform(UniformFormat format, const char* name, uint dataSize, void* data)
	{
		assert(numUniforms < UNIFORMS_MAX_CAPACITY );

		uniforms[numUniforms].initialize(format, name, dataSize, data);
		return &(uniforms[numUniforms++]);
	}

	#pragma endregion

	#pragma region ShaderInfo

	void Renderer::ShaderInfo::initialize(uint programIDIn, uint vertexIDIn, uint fragmentIDIn, uint geometryShaderIDIn)
	{
		programID = programIDIn;
		vertexID = vertexIDIn;
		fragmentID = fragmentIDIn;
		geometryShaderID = geometryShaderIDIn;
	}

	#pragma endregion

	#pragma region Renderable
	
	void Renderer::Renderable::initialize(Renderer* parent, uint geometryIDIn, uint programIndexIn, bool visibleIn, bool usesDepthTest)
	{
		this->parent = parent;
		geometryID = geometryIDIn;
		programIndex = programIndexIn;

		numUniforms = 0;
		numUniformBlocks = 0;
		numTextures = 0;

		visible = visibleIn;
		useDepthTest = usesDepthTest;
	}

	Renderer::UniformInfo* Renderer::Renderable::addUniform(UniformFormat formatIn, const char* name, uint dataSize, void* data)
	{
		assert(numUniforms < UNIFORMS_MAX_CAPACITY );

		uniforms[numUniforms].initialize(formatIn, name, dataSize, data);
		return &(uniforms[numUniforms++]);
	}

	uint Renderer::Renderable::addTexture(uint textureID, const char* nameInShader)
	{		
		Renderer::TextureInfo& tex = textureInfos[numTextures];
		tex.textureID = textureID;
		tex.uniformLoc = glGetUniformLocation(parent->shaderPrograms[programIndex].programID, nameInShader);
		return numTextures++;
	}

	Renderer::UniformBlockInfo* Renderer::Renderable::addUniformBlock(const char* name, uint objectSize, uint numObjects, void* data)
	{
		assert( objectSize * numObjects <= GL_MAX_UNIFORM_BLOCK_SIZE);
		
		parent->insertIntoOneBuffer(GL_UNIFORM_BUFFER, parent->nextUniformOffset, &(parent->currentUniformBufferIndex),
			UNIFORMBUFFERIDS_MAX_CAPACITY, numObjects*objectSize, parent->uniformBufferIDs, data);

		uniformBlocks[numUniformBlocks].initialize(objectSize, numObjects,
			parent->uniformBufferIDs[parent->currentUniformBufferIndex], parent->nextUniformOffset[parent->currentUniformBufferIndex]);

		Renderer::UniformBlockInfo* currentBlock = &(uniformBlocks[numUniformBlocks]);
		
		uint realProgram = parent->shaderPrograms[programIndex].programID;

		uint bindingPoint = parent->nextAvailableUniformBlockBindingPoint;
		uint blockIndex = glGetUniformBlockIndex(realProgram, name);
		glUniformBlockBinding(realProgram, blockIndex, bindingPoint);

		glBindBuffer(GL_UNIFORM_BUFFER, currentBlock->uBuffer);
		uint chunkSize = currentBlock->numObjects * currentBlock->objectSize;
		glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, currentBlock->uBuffer, currentBlock->uBufferOffset, chunkSize);

		int offsetVal;
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &offsetVal);
		
		parent->nextUniformOffset[parent->currentUniformBufferIndex] += chunkSize;
		parent->nextUniformOffset[parent->currentUniformBufferIndex] += offsetVal - (parent->nextUniformOffset[parent->currentUniformBufferIndex] % offsetVal);

		//uint x = glGetError();

		//char* buf = new char[chunkSize];
		//glGetBufferSubData(GL_UNIFORM_BUFFER, parent->nextUniformOffset[parent->currentUniformBufferIndex], chunkSize, buf);

		/*if(parent->nextAvailableUniformBlockBindingPoint == 2)
		{
		std::ofstream out("rawOutput", std::ios::out | std::ios::binary);

		out.write(reinterpret_cast<char*>(&x), sizeof(uint));
		x = currentBlock->uBufferOffset;
		out.write(reinterpret_cast<char*>(&x), sizeof(uint));
		x = GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT;
		out.write(reinterpret_cast<char*>(&x), sizeof(uint));
		out.close();
		}*/
		
		numUniformBlocks++;

		parent->nextAvailableUniformBlockBindingPoint++;

		return currentBlock;
	}

	#pragma endregion

	#pragma region UniformBufferInfo

	void Renderer::UniformBlockInfo::initialize(uint objectSize, uint numObjects, uint uBuffer, uint uBufferOffset)
	{
		this->objectSize = objectSize;
		this->numObjects = numObjects;
		this->uBuffer = uBuffer;
		this->uBufferOffset = uBufferOffset;
	}

	void Renderer::UniformBlockInfo::getObjectCopy(uint indexOfObject, void* objectBuffer)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, uBuffer);
		uint offsetIntoBlock = objectSize * indexOfObject;
		glGetBufferSubData(GL_UNIFORM_BUFFER, uBufferOffset + offsetIntoBlock, objectSize, objectBuffer);
	}

	void Renderer::UniformBlockInfo::updateObject(uint indexOfObject, void* newObjectData)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, uBuffer);
		uint offsetIntoBlock = objectSize * indexOfObject;
		glBufferSubData(GL_UNIFORM_BUFFER, uBufferOffset + offsetIntoBlock, objectSize, newObjectData);
	}

	void Renderer::UniformBlockInfo::getMemberCopy(uint indexOfObject, uint offsetIntoObject, uint size, void* data)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, uBuffer);
		uint offsetIntoBlock = objectSize * indexOfObject + offsetIntoObject;
		glGetBufferSubData(GL_UNIFORM_BUFFER, uBufferOffset + offsetIntoBlock, size, data);
	}

	void Renderer::UniformBlockInfo::updateMember(uint indexOfObject, uint offsetIntoObject, uint size, void* data)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, uBuffer);
		uint offsetIntoBlock = objectSize * indexOfObject + offsetIntoObject;
		glBufferSubData(GL_UNIFORM_BUFFER, uBufferOffset + offsetIntoBlock, size, data);
	}

	

	#pragma endregion

	#pragma endregion

	#pragma region Override Functions (GL init and paint)
	
	void Renderer::initializeGL()
	{
		glewInit();
		show();

		glClearColor(0.0f,0.0f,0.0f,0.0f);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		initializeSize();

		camera.initialize(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		projection = glm::perspective(60.0f, ((float)width()/height()),
		0.1f, 200.0f);
	}

	void Renderer::paintGL()
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		update();
	}

	#pragma endregion

	#pragma region Event Handlers

	void Renderer::keyPressEvent(QKeyEvent* )
	{
	}
	
	void Renderer::mousePressEvent(QMouseEvent* e)
	{
		lastX = e->x();
		lastY = e->y();
		glDraw();
	}

	void Renderer::resizeEvent(QResizeEvent*)
	{
		initializeSize();
	}

	#pragma endregion

	#pragma region Buffer and Shader Functions

	void Renderer::compileShader(const char* fileName, GLuint shaderID)  
	{
		QFile input(fileName);
		
		if( ! input.open(QIODevice::ReadOnly | QIODevice::Text) )
		{
			qDebug() << "File failed to open: " << fileName;
			exit(1);
		}

		QTextStream stream(&input);
		QString shaderCodeText = stream.readAll();
		input.close();

		std::string temp = shaderCodeText.toStdString();
		const char* shaderSource = temp.c_str();

		const char* buff[1];
		buff[0] = shaderSource;
		glShaderSource(shaderID, 1, buff, NULL);
		glCompileShader(shaderID);

		GLint status;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
		if(status != GL_TRUE)
		{
			GLint infoLogLen;
			glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLen);

			char* buffer = new char[infoLogLen];

			GLsizei BitBucket;

			glGetShaderInfoLog(shaderID, infoLogLen, &BitBucket, buffer);

			qDebug() << buffer;

			delete [] buffer;

			exit(1);
		}
	}

	void Renderer::insertIntoOneBuffer(uint bindingTarget, uint* nextOffset, uint* currentBufferIndex, uint maxBuffers, uint bufferSize, uint* bufferIDs, void* data)
	{
		if(nextOffset[*currentBufferIndex] + bufferSize > BUFFER_SIZE) // The current buffer can't fit the data. Use the next one.
		{
			(*currentBufferIndex)++;
			nextOffset[*currentBufferIndex] = 0;
		}

		assert( *currentBufferIndex < maxBuffers );

		if(nextOffset[*currentBufferIndex] <= 0) // due to architecture, if the buffer offset is 0 then the corresponding buffer has not been created by gl
		{
			if(bufferSize > BUFFER_SIZE) // special buffer creation for large data
			{
				createBuffer(
					&(bufferIDs[*currentBufferIndex]),
					bindingTarget, bufferSize, GL_STATIC_DRAW);
				
				glBufferSubData(bindingTarget, NULL, bufferSize, data);
			}
			else // normal buffer creation
			{
				createBuffer(&(bufferIDs[*currentBufferIndex]), bindingTarget, BUFFER_SIZE, GL_STREAM_DRAW);
			}
		}

		assert( currentGeometriesIndex < GEOMETRIES_MAX_CAPACITY );

		if(nextOffset[*currentBufferIndex] + bufferSize <= BUFFER_SIZE) // this prevents mishandling of data larger than BUFFER_SIZE
		{
			glBufferSubData(bindingTarget, nextOffset[*currentBufferIndex], bufferSize, data);
		}
	}

	void Renderer::createBuffer(uint* bufferID, uint GLbindingTarget, uint size, uint GLusage)
	{
		glGenBuffers(1, bufferID);
		glBindBuffer(GLbindingTarget, *bufferID);
		glBufferData(GLbindingTarget, size, NULL, GLusage);
	}

	#pragma endregion

	#pragma region Add Component Functions

	uint Renderer::addTexture(const char* fileName, const char* imageExt)
	{
		QImage texture =
		QGLWidget::convertToGLFormat(QImage(fileName, imageExt));
		glActiveTexture(GL_TEXTURE0);
		
		glGenTextures(1, textures+nextTextureIndex);
		glBindTexture(GL_TEXTURE_2D, textures[nextTextureIndex]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width(),
		texture.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
		texture.bits());
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR);
		return nextTextureIndex++;
	}

	

	uint Renderer::addGeometry(uint vertexBufferSize, uint indexBufferSize, uint numIndices, void* verts, void* indices, uint stride, uint numVAttribs, uint* sizes, uint* offsets, DrawStyle drawStyleIn)
	{
		assert( currentGeometriesIndex < GEOMETRIES_MAX_CAPACITY );

		insertIntoOneBuffer(GL_ARRAY_BUFFER, nextVertexOffset, &currentVertexBufferIndex, VERTEXBUFFERIDS_MAX_CAPACITY, vertexBufferSize, vertexBufferIDs, verts);
		insertIntoOneBuffer(GL_ELEMENT_ARRAY_BUFFER, nextIndexOffset, &currentIndexBufferIndex, INDEXBUFFERIDS_MAX_CAPACITY, indexBufferSize, indexBufferIDs, indices);
		
		geometries[currentGeometriesIndex].initialize(
			vertexBufferIDs[currentVertexBufferIndex], indexBufferIDs[currentIndexBufferIndex],
			nextVertexOffset[currentVertexBufferIndex], nextIndexOffset[currentIndexBufferIndex],
			numIndices, stride, numVAttribs, sizes, offsets, drawStyleIn);

		nextVertexOffset[currentVertexBufferIndex] += vertexBufferSize;
		nextIndexOffset[currentIndexBufferIndex] += indexBufferSize;
		
		return currentGeometriesIndex++;
	}

	Renderer::BufferedGeometryInfo* Renderer::addBufferedGeometry(uint numVerts, uint shaderID, Renderer::DrawStyle drawStyle)
	{
		assert(currentBufferedGeometriesIndex < BUFFEREDGEOMETRIES_MAX_CAPACITY);

		BufferedGeometryInfo& geo = bufferedGeometries[currentBufferedGeometriesIndex];

		geo.shaderID = shaderID;
		geo.numVerts = numVerts;
		geo.drawStyle = drawStyle;

		return bufferedGeometries + (currentBufferedGeometriesIndex++);
	}

	uint Renderer::addProgram(const char* vshFileName, const char* fshFileName)
	{
		assert( currentShaderProgramsIndex < SHADERPROGRAMS_MAX_CAPACITY);

		ShaderInfo& target = shaderPrograms[currentShaderProgramsIndex];

		target.vertexID = glCreateShader(GL_VERTEX_SHADER);
		target.fragmentID = glCreateShader(GL_FRAGMENT_SHADER);

		compileShader(vshFileName, target.vertexID);
		compileShader(fshFileName, target.fragmentID);

		target.programID = glCreateProgram();

		glAttachShader(target.programID, target.vertexID);
		glAttachShader(target.programID, target.fragmentID);
		glLinkProgram(target.programID);

		return currentShaderProgramsIndex++;
	}

	uint Renderer::addProgram(const char* vshFileName, const char* fshFileName, const char* gshFileName)
	{
		assert( currentShaderProgramsIndex < SHADERPROGRAMS_MAX_CAPACITY);

		ShaderInfo& target = shaderPrograms[currentShaderProgramsIndex];

		target.vertexID = glCreateShader(GL_VERTEX_SHADER);
		target.fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
		target.geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

		compileShader(vshFileName, target.vertexID);
		compileShader(fshFileName, target.fragmentID);
		compileShader(gshFileName, target.geometryShaderID);

		target.programID = glCreateProgram();

		glAttachShader(target.programID, target.vertexID);
		glAttachShader(target.programID, target.fragmentID);
		glAttachShader(target.programID, target.geometryShaderID);
		glLinkProgram(target.programID);

		return currentShaderProgramsIndex++;
	}

	Renderer::Renderable* Renderer::addRenderable(uint geometryID, uint shaderProgramIndex, bool usesDepthTest)
	{
		assert( currentRenderablesIndex < RENDERABLES_MAX_CAPACITY);

		renderables[currentRenderablesIndex].initialize(this, geometryID, shaderProgramIndex, true, usesDepthTest);

		return &(renderables[currentRenderablesIndex++]);
	}

	#pragma endregion

	void Renderer::update()
	{
	if(!paused)
	{
		//clock.frameTick();
		drawRenderables(camera.getViewMatrix(), projection);
		drawBufferedGeometries(camera.getViewMatrix(), projection);
		//std::cout << 1.0 / clock.deltaTime() << std::endl;
		POINT globalPos;
		GetCursorPos(&globalPos);
		int globalx = globalPos.x;
		int globaly = globalPos.y;
		QPoint screenPos(mapFromGlobal(QPoint(globalx, globaly)));

		int screenX = screenPos.x();
		int screenY = screenPos.y();
		int moveH = screenX - lastX;
		int moveV = screenY - lastY;
		camera.update(moveH, moveV);
		lastX = screenX;
		lastY = screenY;
	}
	}

	void Renderer::drawRenderables(const glm::mat4& view, const glm::mat4& projection)
	{
		glm::mat4 viewPerspective(projection * view);
		
		for( uint i=0; i < currentRenderablesIndex; i++)
		{
			if(renderables[i].visible)
			{
				
				if(renderables[i].useDepthTest)
					glEnable(GL_DEPTH_TEST);
				else
					glDisable(GL_DEPTH_TEST); 

				ShaderInfo& shader = shaderPrograms[renderables[i].programIndex];
				GeometryInfo& geometry = geometries[renderables[i].geometryID];
				UniformInfo* uniforms = renderables[i].uniforms;

				glUseProgram( shader.programID );
				
				glBindBuffer(GL_ARRAY_BUFFER, geometry.vBuffer);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.iBuffer);
				

				GLint uniformLocation;
				
				for(uint j=0; j<renderables[i].numUniforms; j++)
				{
					uniformLocation = glGetUniformLocation( shader.programID, uniforms[j].name);
					if(uniformLocation >= 0)
					{
						switch(uniforms[j].format)
						{
							case UniformFormat::GLMVEC3:
								glUniform3fv(uniformLocation, 1, reinterpret_cast<float*>(uniforms[j].data));
								break;
							case UniformFormat::GLMVEC4:
								glUniform4fv(uniformLocation, 1, reinterpret_cast<float*>(uniforms[j].data));
								break;
							case UniformFormat::GLMMAT3:
								glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, reinterpret_cast<float*>(uniforms[j].data));
								break;
							case UniformFormat::GLMMAT4:
								glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, reinterpret_cast<float*>(uniforms[j].data));
								break;
							case UniformFormat::FLOAT:
								glUniform1fv(uniformLocation, 1, reinterpret_cast<float*>(uniforms[j].data));
								break;
							case UniformFormat::INT:
								glUniform1iv(uniformLocation, 1, reinterpret_cast<int*>(uniforms[j].data));
								break;
							case UniformFormat::GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE:
								glUniformMatrix4fv( uniformLocation, 1, GL_FALSE, &(viewPerspective * *reinterpret_cast<glm::mat4*>(uniforms[j].data))[0][0] );
								break;
							case UniformFormat::GLMMAT4_AFFECTEDBYVIEW:
								glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &(view * *reinterpret_cast<glm::mat4*>(uniforms[j].data))[0][0]);
								break;
							default:
								qDebug() << "Invalid UniformFormat";
						}
					}
				}

				for(uint j=0; j<geometry.numVAttribs;j++)
				{
					glEnableVertexAttribArray(j);
					glVertexAttribPointer(j, geometry.vAttribs[j].size, GL_FLOAT, GL_FALSE, geometry.stride, BUFFER_OFFSET(geometry.vOffset + geometry.vAttribs[j].offset));
				}

				for(uint k=0; k<renderables[i].numTextures; k++)
				{
					TextureInfo& tex = renderables[i].textureInfos[k];

					glActiveTexture(GL_TEXTURE0 + k);
					glBindTexture(GL_TEXTURE_2D, textures[tex.textureID]);
					glUniform1i(tex.uniformLoc, k);
				}

				for(uint k=0; k<renderables[i].numUniformBlocks; k++)
				{
					UniformBlockInfo& ubi = renderables[i].uniformBlocks[k];
				}

				if(geometry.drawStyle == DrawStyle::LINES)
					glDrawElements(GL_LINES, geometry.numIndices, GL_UNSIGNED_SHORT, BUFFER_OFFSET(geometry.iOffset));
				else if(geometry.drawStyle == DrawStyle::TRIANGLES)
					glDrawElements(GL_TRIANGLES, geometry.numIndices, GL_UNSIGNED_SHORT, BUFFER_OFFSET(geometry.iOffset));
				else if(geometry.drawStyle == DrawStyle::POINT_ARRAY)
					glDrawArrays(GL_POINTS, 0, geometry.numIndices);
			}
		}
	}

	void Renderer::drawBufferedGeometries(const glm::mat4& view, const glm::mat4& projection)
	{
		glm::mat4 viewPerspective(projection * view);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,);

		for(int i=0; i<currentBufferedGeometriesIndex; i++)
		{
			BufferedGeometryInfo& geo = bufferedGeometries[i];
			glUseProgram(geo.shaderID);
			for(int j=0; j<geo.numAttribs; j++)
			{
				glEnableVertexAttribArray(j);
				AttribBufferInfo& attrib = geo.attribs[j];
				//data for an attribBuffer: buffer, size, type, stride
				// data for a BGI: numAttribs, attribs, drawStyle, numVerts
				glBindBuffer(GL_ARRAY_BUFFER, attrib.buffer);
				
				glVertexAttribPointer(j, attrib.size, attrib.glType, GL_FALSE, attrib.stride, 0);
			}

			uint uniformLocation = -1;
			UniformInfo* uniforms = geo.uniforms;
			for(uint j=0; j<geo.numUniforms; j++)
			{
				uniformLocation = glGetUniformLocation( geo.shaderID, uniforms[j].name);
				if(uniformLocation >= 0)
				{
					switch(uniforms[j].format)
					{
						case UniformFormat::GLMVEC3:
							glUniform3fv(uniformLocation, 1, reinterpret_cast<float*>(uniforms[j].data));
							break;
						case UniformFormat::GLMVEC4:
							glUniform4fv(uniformLocation, 1, reinterpret_cast<float*>(uniforms[j].data));
							break;
						case UniformFormat::GLMMAT3:
							glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, reinterpret_cast<float*>(uniforms[j].data));
							break;
						case UniformFormat::GLMMAT4:
							glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, reinterpret_cast<float*>(uniforms[j].data));
							break;
						case UniformFormat::FLOAT:
							glUniform1fv(uniformLocation, 1, reinterpret_cast<float*>(uniforms[j].data));
							break;
						case UniformFormat::INT:
							glUniform1iv(uniformLocation, 1, reinterpret_cast<int*>(uniforms[j].data));
							break;
						case UniformFormat::GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE:
							glUniformMatrix4fv( uniformLocation, 1, GL_FALSE, &(viewPerspective * *reinterpret_cast<glm::mat4*>(uniforms[j].data))[0][0] );
							break;
						case UniformFormat::GLMMAT4_AFFECTEDBYVIEW:
							glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &(view * *reinterpret_cast<glm::mat4*>(uniforms[j].data))[0][0]);
							break;
						default:
							qDebug() << "Invalid UniformFormat";
					}
				}
			}

			//glm::vec4* positionsTemp = new glm::vec4[8192];
			//glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4)*8192, positionsTemp);
			glPointSize(pointSize);
			glDrawArrays(GL_POINTS, 0, geo.numVerts);
			//glDrawElements(GL_POINTS, geo.numVerts, GL_UNSIGNED_SHORT, 0);
		}
	}

	void Renderer::initializeSize()
	{
		glViewport(0,0,width(), height());
		projection = glm::perspective(60.0f, ((float)width()/height()),
			0.1f, 200.0f);
		
	}

	#pragma region Debug
		#ifdef ENGINE_DEBUG
			void Renderer::debug_setCamera(Camera* cameraIn)
			{
				debugCamera = cameraIn;
			}

			Renderer::GeometryInfo* Renderer::debug_getGeometryInfoAtIndex(uint geometryID)
			{
				return &geometries[geometryID];
			}

			Renderer::Renderable* Renderer::debug_getRenderables()
			{
				return renderables;
			}
		#endif
	#pragma endregion
}