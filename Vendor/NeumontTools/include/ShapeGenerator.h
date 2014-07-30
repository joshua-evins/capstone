#ifndef SHAPE_GENERATOR_H
#define SHAPE_GENERATOR_H
#include "glm\glm.hpp"
#include "TypeDefs.h"
#include "ShapeData.h"
#include "ExportImportHeader.h"

namespace Neumont
{
	// After you call any function in this class, you are responsible to delete
	// all the data referenced by the returned ShapeData. You can call
	// ShapeData::cleanUp() to do this for you if you like.
	class DLL_SHARED ShapeGenerator
	{
	public:
		// Makes a line from (-1, 0, 0) to (1, 0, 0)
		// Must render with GL_LINES
		static ShapeData makeLine();
		// Render GL_TRIANGLES
		static ShapeData makePlane(uint dimensions);
		// Render GL_LINES
		static ShapeData makeWireframePlane(uint dimensions);
		static ShapeData makeCube();
		static ShapeData makeSphere(uint tesselation);
		static ShapeData makeTorus(uint tesselation);
		static ShapeData makeArrow();
		static ShapeData makeTeapot(uint tesselation, const glm::mat4& lidTransform);
		//Makes a circle around the Z-axis with radius 1
		//Makes a triangle when tesselation = 1
		//Must render with GL_LINES
		static ShapeData makeCircle(uint tesselation);
		//Makes a wireframe cube from (0,0,0) to (1,1,1)
		//Must render with GL_LINES
		static ShapeData ShapeGenerator::makeWireFrameCube();
		// Generates a ShapeData for GL_LINES that will render 
		// white normals for the given ShapeData. Only the Vertex 
		// positions and colors will have any meaning
		static ShapeData generateNormals(const ShapeData& data);
		static ShapeData makeVector(uint tesselation);
		static ShapeData makeCone(uint tesselation);
		static ShapeData makeCylinder(uint tesselation);
	private:
		static void makeTeapot(
			unsigned int grid, const glm::mat4& lidTransform,
			unsigned int& numVerts,
			float*& verts,
			float*& normals, 
			float*& textureCoordinates,
			unsigned short*& indices, 
			unsigned int& numIndices);

		static ShapeData makePlaneVerts(uint tesselation);
		static ShapeData makePlaneIndices(uint tesselation);
		static ShapeData makePlaneUnseamedIndices(uint tesselation);

		// Teapot:
		static void generatePatches(float * v, float * n, float *tc, unsigned short* el, int grid);
		static void buildPatchReflect(int patchNum,
			float *B, float *dB,
			float *v, float *n, float *, unsigned short *el,
			int &index, int &elIndex, int &, int grid,
			bool reflectX, bool reflectY);
		static void buildPatch(glm::vec3 patch[][4],
			float *B, float *dB,
			float *v, float *n,float *, unsigned short *el,
			int &index, int &elIndex, int &, int grid, glm::mat3 reflect, bool invertNormal);
		static void getPatch( int patchNum, glm::vec3 patch[][4], bool reverseV );
		static void computeBasisFunctions( float * B, float * dB, int grid );
		static glm::vec3 evaluate( int gridU, int gridV, float *B, glm::vec3 patch[][4] );
		static glm::vec3 evaluateNormal( int gridU, int gridV, float *B, float *dB, glm::vec3 patch[][4] );
		static void moveLid(int,float *,glm::mat4);
	};
}
#endif