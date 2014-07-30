#include "FbxLoader.h"
#include <vector>
#include "FbxExtractor.h"
#include "MyVertex.h"
#include "../AnimationKeySet.h"
#include "../Vertex.h"
#include <iostream>
using std::cout;
using std::endl;

namespace Neumont
{
	bool FbxLoader::averageNormals = true;

	void AverageNormals(Neumont::ShapeData& outData);
	void FbxLoader::ToggleAverage(bool toggle)
	{
		averageNormals = toggle;
	}

	bool FbxLoader::loadFbxFile(
		const char* filename, 
		Neumont::ShapeData*& shapeDatas,
		unsigned int& numShapeDatas)
	{
		// I know this can be cleanup up, especially ownership
		std::vector<GeometryToolData<MyVertex>> geometryToolData;
		std::vector<std::string> textureFileNames;
		std::vector<ushort> animIndicies;
		std::vector<AnimationKeyFrameSet> keySet;
		std::vector<skeletonBone> skeleton;
		FbxExtractor<MyVertex> extractor;
		bool b = extractor.pullScene(filename, geometryToolData, textureFileNames,animIndicies, keySet,skeleton);
		if(! b )
			return false;

		numShapeDatas = geometryToolData.size();
		shapeDatas = new ShapeData[numShapeDatas];

		for(int shapeDataIndex = 0; shapeDataIndex < numShapeDatas; shapeDataIndex++)
		{
			GeometryToolData<MyVertex>& data = geometryToolData[shapeDataIndex];
			Neumont::ShapeData& outData = shapeDatas[shapeDataIndex];

			////////outData.animationKeySet = toArray(keySet);

			outData.numVerts = data.numVerts;
			outData.numIndices = data.numIndices;

			outData.verts = new Neumont::Vertex[data.numVerts * 4];
			for(int i = 0; i < data.numVerts; i++)
			{
				const MyVertex& source = data.vertices[i];
				Neumont::Vertex& target = outData.verts[i];
				target.position.x = source.x;
				target.position.y = source.y;
				target.position.z = source.z;

				// Convert color range from 0-255 int to 0-1 float
				//switched to float from 0-255
				unsigned int color = source.color;
				target.color.r = ((color & 0xff000000)>> 24) / 1.0f;
				target.color.g = ((color & 0x00ff0000)>> 16) / 1.0f;
				target.color.b = ((color & 0x0000ff00)>> 8) / 1.0f; 
				target.color.a = 255;

				target.normal.x = source.normal.x;
				target.normal.y = source.normal.y;
				target.normal.z = source.normal.z;

				target.uv.x = source.u;
				target.uv.y = source.v;

			}
			outData.indices = data.indices; // Caller takes ownership
			delete [] data.vertices;

			outData.textureFileName = NULL;
			outData.normalMapFileName = NULL;
			uint check = -1;
			if(data.textureIndex != check)
			{
				outData.textureFileName = new char[textureFileNames[data.textureIndex].length()+1];
				strcpy(outData.textureFileName, textureFileNames[data.textureIndex].c_str());
			}
			else
			{
				outData.textureFileName = NULL;
			}
			if(data.normalMapIndex != check)
			{
				outData.normalMapFileName = new char[textureFileNames[data.normalMapIndex].length()+1];
				strcpy(outData.normalMapFileName, textureFileNames[data.normalMapIndex].c_str());
			}
			else
			{
				outData.normalMapFileName = NULL;
			}
			outData.numAnimations = keySet.size();
			if(outData.numAnimations != 0)
			{
				outData.animation = new AnimationKeyFrameSet[keySet.size()];
			}
			else
			{
				outData.animation = NULL;
			}
			if(outData.numAnimations !=0)
			{
				outData.frameCount = keySet[0].quatKeys.size();
			}
			else
			{
				outData.frameCount =0;
			}
			for(int i=0;i<keySet.size();i++)
			{
				outData.animation[i] = keySet[i];
			}
			outData.skeleton = new skeletonBone[skeleton.size()];
			outData.numBones = skeleton.size();
			for(int i=0;i<skeleton.size();i++)
			{
				outData.skeleton[i] = skeleton[i];
			}
			if(averageNormals)
				AverageNormals(outData);
		}
		return true;
	}


	void AverageNormals(Neumont::ShapeData& outData)
	{
		//vector for storing the new set of verts
		std::vector<Neumont::Vertex> verts;
		for(int i= 0; i < outData.numVerts; i++)
		{
			verts.push_back(outData.verts[i]);
		}

		//first iteration through to average normals
		for(int i= 0; i < verts.size(); i++)
		{
			int count =0;
			float* normals = new float[outData.numVerts * 3];
			for(int j = i+1; j < outData.numVerts;j++)
			{
				if(verts[i].position.x == outData.verts[j].position.x &&
					verts[i].position.y == outData.verts[j].position.y &&
					verts[i].position.z == outData.verts[j].position.z)
				{
					normals[j *3] = outData.verts[j].normal.x;
					normals[j *3 +1] = outData.verts[j].normal.y;
					normals[j *3 +2] = outData.verts[j].normal.z;
					count++;
				}
			}
			
			for(int j=0;j<count;j++)
			{
				normals[0]+= normals[(j+1)*3];
				normals[1]+= normals[(j+1)*3+1];
				normals[2]+= normals[(j+1)*3+2];
			}
			outData.verts[i].normal.x = normals[0]/count;
			outData.verts[i].normal.y = normals[1]/count;
			outData.verts[i].normal.z = normals[2]/count;
			delete normals;
		}

		//second iteration through to delete duplicate verts
		for(int i=0;i<verts.size();i++)
		{
			for(int j=0;j<verts.size();j++)
			{
				if( i != j && verts[i].position.x == verts[j].position.x &&
					verts[i].position.y == verts[j].position.y &&
					verts[i].position.z == verts[j].position.z)
				{
					vector<Neumont::Vertex>::iterator where = verts.begin() + j;
					verts.erase(where);
					j--;
				}
			}
		}
		//third iteration to fix indicies
		for(int i=0;i<verts.size();i++)
		{
			for(int j=0;j<outData.numVerts;j++)
			{
				if(verts[i].position.x == outData.verts[j].position.x &&
					verts[i].position.y == outData.verts[j].position.y &&
					verts[i].position.z == outData.verts[j].position.z)
				{
					for(int x=0;x<outData.numIndices;x++)
					{
						if(outData.indices[x] == j)
							outData.indices[x] = i;
					}
				}
			}
		}

		outData.numVerts = verts.size();
		outData.verts = toArray(verts);
	}
}