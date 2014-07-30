#include "fbx\FbxLoader.h"

void main()
{
	Neumont::ShapeData data;
	Neumont::FbxLoader::ToggleAverage(false);
	Neumont::ShapeData* geometries;
	unsigned int numGeometries;
    //Neumont::FbxLoader::loadFbxFile("c:\\NeumontSeperated.fbx", geometries, numGeometries);
    //Neumont::FbxLoader::loadFbxFile("c:\\building.fbx", geometries, numGeometries);
	//Neumont::FbxLoader::loadFbxFile("c:\\NeumontBareWithBumpMap.fbx", geometries, numGeometries);
	Neumont::FbxLoader::loadFbxFile("mikuModel.fbx",geometries,numGeometries);
}