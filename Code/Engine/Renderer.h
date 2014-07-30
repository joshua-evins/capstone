#ifndef RENDERER_H
#define RENDERER_H
#include "typedefs_Engine.h"
#include <GL\glew.h>
#include <glm\glm.hpp>
#include <QtOpenGL\qglwidget>
#include <Qt\qobject.h>
#include "ExportHeader.h"
#include "Camera.h"
#include "Clock.h"

namespace Engine
{
	class ENGINE_SHARED Renderer : public QGLWidget
	{
	#pragma region Instance Variables
	private:
		Clock clock;
		
		int lastX;
		int lastY;

		uint nextAvailableUniformBlockBindingPoint;

		uint currentGeometriesIndex;
		uint currentBufferedGeometriesIndex;
		uint currentShaderProgramsIndex;
		uint currentRenderablesIndex;

		uint vertexBufferIDs[VERTEXBUFFERIDS_MAX_CAPACITY];
		uint currentVertexBufferIndex;
		uint nextVertexOffset[VERTEXBUFFERIDS_MAX_CAPACITY];
		
		uint indexBufferIDs[INDEXBUFFERIDS_MAX_CAPACITY];
		uint currentIndexBufferIndex;
		uint nextIndexOffset[INDEXBUFFERIDS_MAX_CAPACITY];

		uint uniformBufferIDs[UNIFORMBUFFERIDS_MAX_CAPACITY];
		uint currentUniformBufferIndex;
		uint nextUniformOffset[UNIFORMBUFFERIDS_MAX_CAPACITY];

		uint nextTextureIndex;
		uint textures[TEXTURES_MAX_CAPACITY];
	public:
		bool paused;
		glm::mat4 projection;
		Camera camera;
		int pointSize;
	#pragma endregion

		Renderer();

	#pragma region Enums
	public:
		enum UniformFormat
		{
			GLMVEC3,
			GLMVEC4,
			GLMMAT3,
			GLMMAT4,
			GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE,
			GLMMAT4_AFFECTEDBYVIEW,
			FLOAT,
			INT
		};

		enum DrawStyle
		{
			LINES,
			TRIANGLES,
			POINT_ARRAY
		};
	#pragma endregion

	#pragma region Internal Structs (with variables of that type)
	private:
		struct VAttribInfo
		{
			uint size;
			uint offset;
		};
	public:
		struct ENGINE_SHARED UniformInfo
		{
			UniformFormat format;
			char name[UNIFORMNAME_MAX_CAPACITY];
			char data[UNIFORMDATA_MAX_CAPACITY];

			void updateInfo(uint dataSize, void* data);
			void initialize(UniformFormat formatIn, const char* nameIn, uint dataSize, void* dataIn);
		};

		struct ENGINE_SHARED UniformBlockInfo
		{
			uint uBuffer;
			uint uBufferOffset;
			uint objectSize;
			uint numObjects;

			void initialize(uint objectSize, uint numObjects, uint uBuffer, uint uBufferOffset);
			void getObjectCopy(uint indexOfObject, void* objectBuffer);
			void updateObject(uint indexOfObject, void* newObjectData);

			void getMemberCopy(uint indexOfObject, uint offsetIntoObject, uint size, void* data);
			void updateMember(uint indexOfObject, uint offsetIntoObject, uint size, void* data);
			
		};

		struct ENGINE_SHARED GeometryInfo
		{
			uint vBuffer;
			uint iBuffer;

			uint vOffset;
			uint iOffset;

			uint numIndices;

			uint stride;
			uint numVAttribs;
			VAttribInfo vAttribs[ATTRIBS_MAX_CAPACITY];

			DrawStyle drawStyle;
			
			void initialize(uint vBuffer, uint iBuffer, uint vOffset, uint iOffset, uint numIndices, uint stride, uint numVAttribs, uint* sizes, uint* offsets, DrawStyle drawStyle);
		} geometries[GEOMETRIES_MAX_CAPACITY];

		struct ENGINE_SHARED AttribBufferInfo
		{
			uint buffer;
			uint size;
			uint glType;
			uint stride;
		};

		struct ENGINE_SHARED BufferedGeometryInfo
		{
			BufferedGeometryInfo();

			uint shaderID;
			uint numVerts;
			uint numAttribs;
			AttribBufferInfo attribs[BUFFEREDGEOMETRY_MAX_ATTRIBS];
			DrawStyle drawStyle;

			uint numUniforms;
			UniformInfo uniforms[UNIFORMS_MAX_CAPACITY];

			uint addAttribBuffer(uint numGlTypes, uint glType, uint stride, void* data, uint dataSize, uint usageHint);
			uint addAttribBuffer(uint numGlTypes, uint glType, uint stride, void* data, uint dataSize);

			UniformInfo* addUniform(UniformFormat format, const char* name, uint dataSize, void* data);

		}bufferedGeometries[BUFFEREDGEOMETRIES_MAX_CAPACITY];
		
		struct ShaderInfo
		{
			uint programID;
			uint vertexID;
			uint fragmentID;
			uint geometryShaderID;
			
			void initialize(uint programIDIn, uint vertexIDIn, uint fragmentIDIn, uint geometryShaderIDIn);
		} shaderPrograms[SHADERPROGRAMS_MAX_CAPACITY];
		
		struct ENGINE_SHARED TextureInfo
		{
			uint textureID;
			uint uniformLoc;
		};

		struct ENGINE_SHARED Renderable
		{
			bool visible;
			Renderer* parent;
			
			uint geometryID;
			uint programIndex;

			uint numUniforms;
			UniformInfo uniforms[UNIFORMS_MAX_CAPACITY];
			uint numUniformBlocks;
			UniformBlockInfo uniformBlocks[UNIFORMBLOCKS_MAX_CAPACITY];
			
			uint numTextures;
			TextureInfo textureInfos[TEXTUREINFOS_MAX_CAPACITY];

			bool useDepthTest;

			void initialize(Renderer* parent, uint geometryID, uint programIndex, bool visible, bool usesDepthTest);
			UniformInfo* addUniform(UniformFormat format, const char* name, uint dataSize, void* data);
			uint addTexture(uint textureID, const char* nameInShader);
			UniformBlockInfo* addUniformBlock(const char* name, uint objectSize, uint numObjects, void* data);
		} renderables[RENDERABLES_MAX_CAPACITY];
	#pragma endregion
		
	private:
	#pragma region Override Functions
		void initializeGL();
		void paintGL();
	#pragma endregion

	private:
	#pragma region Event Handlers
		void keyPressEvent(QKeyEvent *event);
		//void mouseMoveEvent(QMouseEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void resizeEvent(QResizeEvent*);
	#pragma endregion
	
	private:
	#pragma region Buffer and Shader Functions
		void compileShader(const char* fileName, GLuint shaderID);
		void insertIntoOneBuffer(uint bindingTarget, uint* nextOffset, uint* currentBufferIndex, uint maxBuffers, uint bufferSize, uint* bufferIDs, void* data);
		void createBuffer(uint* bufferID, uint GLbindingTarget, uint size, uint GLusage);
	#pragma endregion
		
	public:
	#pragma region Add Component Functions
		uint addGeometry(uint vertexBufferSize, uint indexBufferSize, uint numIndices, void* verts, void* indices,
			uint stride, uint numVAttribs, uint* sizes, uint* offsets, DrawStyle drawStyle);
		BufferedGeometryInfo* addBufferedGeometry(uint numVerts, uint shaderID, Renderer::DrawStyle drawStyle);
		uint addProgram(const char* vshFileName, const char* fshFileName);
		uint addProgram(const char* vshFileName, const char* fshFileName, const char* gshFileName);
		Renderable* addRenderable(uint geometryID, uint shaderProgramID, bool usesDepthTest);
		uint addTexture(const char* fileName, const char* imageExt);
	#pragma endregion

	public:
		void update();
		void drawRenderables(const glm::mat4& view, const glm::mat4& projection);
		void drawBufferedGeometries(const glm::mat4& view, const glm::mat4& projection);
		void initializeSize(); // Used to set the glViewport to the size of the Renderer

	#pragma region Debug
	#ifdef ENGINE_DEBUG
		Camera* debugCamera;
		bool debug_HalfScreen;
		void debug_setCamera(Camera* debugCamera);
		GeometryInfo* debug_getGeometryInfoAtIndex(uint gemoetryID);
		Renderable* debug_getRenderables();
	#endif
	#pragma endregion
	};
}
#endif