#ifndef FBX_EXTRACTOR_H
#define FBX_EXTRACTOR_H
#include "../AnimationKeySet.h"
#include <cassert>
#include "SDK_Utility.h"
#include "GeometryToolData.h"
#include "fbxsdk\fileio\fbxiobase.h"
#include <string>
#include <vector>
#include <iostream>
#include <set>
using std::vector;

template<class T>
T* toArray(const std::vector<T>& source)
{
	T* ret = new T[source.size()];
	for(size_t i = 0; i < source.size(); i++)
		ret[i] = source[i];
	return ret;
}


template <class TVert>
class FbxExtractor
{
	std::vector<GeometryToolData<TVert>> geometries;
	std::vector<std::string> textureFileNames;
	std::vector<AnimationKeyFrameSet> animKeySet;
	struct controlPointWeightMap{
		uint index;
		char* boneName;
		float weight;
		uint boneID;
		controlPointWeightMap(const int& id,char* bone,const float& weit):index(id),boneName(bone),weight(weit),boneID(0){}
	};
	struct vertexWeight{
		float weight;
		uint boneID;
		uint vertexID;
		vertexWeight(uint vid,uint bid, float weit): vertexID(vid),boneID(bid), weight(weit){}
	};
	struct vertexWeightData{
		uint boneID;
		glm::vec3 weight;
		vertexWeightData():boneID(0),weight(0.0f,0.0f,0.0f){}
	};
	std::vector<skeletonBone> skeleton;
	std::vector<uint> vertexMap;
	std::vector<controlPointWeightMap> weightMap;
	std::vector<vertexWeightData> vertexWeights;

	// Adds the indices to the MeshVector, calculating "fanning"
	// quads or polygons out from a single vertex
	void addIndices(size_t numVertsThisPolygon, 
		const std::vector<TVert>& verts,
		std::vector<unsigned short>& indices);
	void pullPosition(KFbxVector4* controlPoints, int controlPointIndex, TVert& vert);
	void pullUVs(KFbxMesh* mesh, int vertexId, int controlPointIndex, 
		int polygonVertex, int vertexIndex, TVert& vert);
	void pullTextureFilenameIndex(KFbxFileTexture* pTexture, int pBlendMode, size_t& textureDataIndex);
	void pullTextureFile(KFbxTexture* pTexture, int pBlendMode, size_t& textureDataIndex);
	void pullTextureInfo(KFbxGeometry* pGeometry, GeometryToolData<TVert>& meshInfo);
	void pullFbxData(const char* nodeName, KFbxMesh* mesh);
	void pullNodeInfo(KFbxNode* node);
	void pullSkinInfo(KFbxMesh* mesh);
	bool isDuplicateMesh(const GeometryToolData<TVert>& data) const;
	bool ExtractAnimation(KFbxScene * pScene,KFbxNode* node);
	void AnimationTake(FbxAnimLayer* pAnimLayer,FbxNode* pNode, KFbxScene * pScene);
	bool AnimationExists(FbxAnimLayer *pAnimLayer,KFbxNode* node);
	void addWeights(vector<TVert>& verts);
	void pullSkeletonInfo(KFbxNode* node);

public:
	bool pullScene(	const char* fileName,
		std::vector<GeometryToolData<TVert>>& baseGeometries,
		std::vector<std::string>& textureFileNames,
		std::vector<ushort>& animIndicies,
		std::vector<AnimationKeyFrameSet>& animationKeySet,
		std::vector<skeletonBone>& skeleton);
};

template<class TCollectionType, class UReturnType>
bool pullT(KFbxMesh* mesh, int vertexId, int controlPointIndex, 
	const TCollectionType* (KFbxLayer::*getCollectionMethod)() const,
	UReturnType& returnVal)
{
	const TCollectionType* collection = NULL;
	for (int j = 0; j < mesh->GetLayerCount() && collection == NULL; j++)
		collection = (mesh->GetLayer(j)->*getCollectionMethod)();
	
	UReturnType sourceValue;

	if(collection == NULL)
		return false;
	
	KFbxLayerElement::EMappingMode mappingMode = collection->GetMappingMode();
	//collection->SetMappingMode( KFbxLayerElementNormal::eByControlPoint);

	switch (mappingMode)
	{
	case KFbxLayerElement::eByControlPoint:
		switch (collection->GetReferenceMode())
		{
		case KFbxLayerElement::eDirect:
			sourceValue = collection->GetDirectArray().GetAt(controlPointIndex);
			break;
		case KFbxLayerElement::eIndexToDirect:
			{
				int id = collection->GetIndexArray().GetAt(controlPointIndex);
				sourceValue = collection->GetDirectArray().GetAt(id);
			}
			break;
		}
		break;

	case KFbxLayerElement::eByPolygonVertex:
		{
			switch (collection->GetReferenceMode())
			{
			case KFbxLayerElement::eDirect:
				sourceValue = collection->GetDirectArray().GetAt(vertexId);
				break;
			case KFbxLayerElement::eIndexToDirect:
				{
					int id = collection->GetIndexArray().GetAt(vertexId);
					sourceValue = collection->GetDirectArray().GetAt(id);
				}
				break;
			}
		}
		break;
	}
	returnVal = sourceValue;
	return true;
}

// Adds the indices to the MeshVector, calculating "fanning"
// quads or polygons out from a single vertex
template <class TVert>
void FbxExtractor<TVert>::addIndices(
	size_t numVertsThisPolygon, 
	const std::vector<TVert>& verts,
	std::vector<unsigned short>& indices)
{
	size_t baseIndex = verts.size() - numVertsThisPolygon;
	for(size_t i = baseIndex + 1; i < baseIndex + numVertsThisPolygon - 1; i++)
	{
		indices.push_back(baseIndex);
		indices.push_back(i);
		indices.push_back(i + 1);
	}
}

template <class TVert>
void FbxExtractor<TVert>::pullPosition(KFbxVector4* controlPoints, int controlPointIndex, TVert& vert)
{
	const KFbxVector4& sourceVert = controlPoints[controlPointIndex];
	vert.x = sourceVert[0];
	vert.y = sourceVert[1];
	vert.z = sourceVert[2];
}

template <class TVert>
void FbxExtractor<TVert>::pullUVs(KFbxMesh* mesh, int vertexId, int controlPointIndex, 
	int polygonVertex, int vertexIndex, TVert& vert)
{
	// Unfortunately, getting UVs is too different 
	// to use my template trick,
	// so some duplicate code in this function:

	KFbxLayerElementUV* leUV = NULL;
	for (int j = 0; j < mesh->GetLayerCount() && leUV == NULL; j++)
		leUV = mesh->GetLayer(j)->GetUVs();
	if(leUV == NULL)
		return;

	KFbxVector2 sourceUV;
	KFbxLayerElement::EMappingMode mappingMode = leUV->GetMappingMode(); 

	switch (mappingMode)
	{
	case KFbxLayerElement::eByControlPoint:
		switch (leUV->GetReferenceMode())
		{
		case KFbxLayerElement::eDirect:
			sourceUV = leUV->GetDirectArray().GetAt(controlPointIndex);
			break;
		case KFbxLayerElement::eIndexToDirect:
			{
				int id = leUV->GetIndexArray().GetAt(controlPointIndex);
				sourceUV = leUV->GetDirectArray().GetAt(id);
			}
			break;
		}
		break;

	case KFbxLayerElement::eByPolygonVertex:
		{
			switch (leUV->GetReferenceMode())
			{
			case KFbxLayerElement::eDirect:
				sourceUV = leUV->GetDirectArray().GetAt(vertexId);
				break;
			case KFbxLayerElement::eIndexToDirect:
				{
					int id = leUV->GetIndexArray().GetAt(vertexId);
					sourceUV = leUV->GetDirectArray().GetAt(id);

				}
				break;
			}
		}
		break;
	}

	vert.u = sourceUV[0];
	vert.v = sourceUV[1];
}

template <class TVert>
void FbxExtractor<TVert>::pullTextureFilenameIndex(KFbxFileTexture* pTexture, int pBlendMode, size_t& textureDataIndex)
{
	const char* fileName = pTexture->GetFileName();

	if( ! fileName)
		return;
	for(size_t i = 0; i < textureFileNames.size(); i++)
		if(textureFileNames[i] == std::string(fileName))
		{
			textureDataIndex = i;
			return;
		}
		textureDataIndex = textureFileNames.size();
		textureFileNames.push_back(fileName);
}

template <class TVert>
void FbxExtractor<TVert>::pullTextureFile(KFbxTexture* pTexture, int pBlendMode, size_t& textureDataIndex)
{
	//pTexture
	textureDataIndex = -1;
}

template <class TVert>
void FbxExtractor<TVert>::pullTextureInfo(KFbxGeometry* pGeometry, GeometryToolData<TVert>& meshInfo)
{
	int lMaterialIndex;
	KFbxProperty kfbxProperty;    
	int numMaterials = pGeometry->GetNode()->GetSrcObjectCount(KFbxSurfaceMaterial::ClassId);
	for (lMaterialIndex = 0; lMaterialIndex < numMaterials; lMaterialIndex++){
		KFbxSurfaceMaterial *material = 
			(KFbxSurfaceMaterial *)pGeometry->GetNode()->
			GetSrcObject(KFbxSurfaceMaterial::ClassId, lMaterialIndex);

		if(! material)
			continue;
		meshInfo.textureIndex = -1;
		for(int lTextureIndex = 0; lTextureIndex < KFbxLayerElement::sTypeTextureCount;lTextureIndex++)
		{
			const char* textureChannelName = KFbxLayerElement::sTextureChannelNames[lTextureIndex];
			kfbxProperty = material->FindProperty(textureChannelName);			
			if(kfbxProperty.IsValid())
			{
				// No layered texture simply get on the property
				int numTextures = kfbxProperty.GetSrcObjectCount(KFbxTexture::ClassId);
				for(int j =0; j < numTextures; ++j)
				{
					//pulls texture
					KFbxTexture* texture = FbxCast <KFbxFileTexture> (
						kfbxProperty.GetSrcObject(KFbxTexture::ClassId, j));
					
					if(texture)
					{
						pullTextureFile(texture, -1, meshInfo.textureIndex);
					}
					//pulls texture file name
					KFbxFileTexture* textureFile = FbxCast <KFbxFileTexture> (
						kfbxProperty.GetSrcObject(KFbxTexture::ClassId, j));
					if(textureFile)
					{
						KString propertyName = kfbxProperty.GetName();
						int textureNum = j;

						FbxLayerElement::EType typeTextureValueOffsetted = ((FbxLayerElement::EType)
							(FbxLayerElement::sTypeTextureStartIndex + lTextureIndex));
						if(typeTextureValueOffsetted == FbxLayerElement::EType::eTextureDiffuse)
							pullTextureFilenameIndex(textureFile, -1, meshInfo.textureIndex);
						else if(typeTextureValueOffsetted == FbxLayerElement::EType::eTextureNormalMap)
							pullTextureFilenameIndex(textureFile, -1, meshInfo.normalMapIndex);
						else 
						{ /*assert(false);*/
							meshInfo.textureIndex = -1;
						} // It's a different texture type than these ones, Look at FbxLayerElement::EType to see other possibilities
					}
				}
			}
		}
	}
}

template <class TVert>
void FbxExtractor<TVert>::pullFbxData(const char* nodeName, KFbxMesh* mesh)
{
	//std::map<uint*> hash_map;
	pullSkinInfo(mesh);
	GeometryToolData<TVert> meshInfo;

	int numPolygons = mesh->GetPolygonCount();

	KFbxVector4* controlPoints = mesh->GetControlPoints(); 

	// Textures
	pullTextureInfo(mesh, meshInfo);

	// Polygons
	int vertexId = 0;

	vector<TVert> vertss;
	vector<unsigned short> indicess;
	for (int polygonIndex = 0; polygonIndex < numPolygons; polygonIndex++)
	{
		int numVertsThisPolygon = mesh->GetPolygonSize(polygonIndex);
		for (int vertexIndex = 0; vertexIndex < numVertsThisPolygon; vertexIndex++)
		{
			//ANIMATIONS
			//use this value to map vertex to wieghts I've pulled so far once I figure out how we want to store them
			int controlPointIndex = mesh->GetPolygonVertex(polygonIndex, vertexIndex);

			TVert vert;
			pullPosition(controlPoints, controlPointIndex, vert);

			KFbxVector4 sourceNormal;
			bool b = pullT<KFbxLayerElementNormal, KFbxVector4>(mesh, vertexId, 
				controlPointIndex, &KFbxLayer::GetNormals, sourceNormal);
			if(b)
			{
				vert.normal.x = sourceNormal[0];
				vert.normal.y = sourceNormal[1];
				vert.normal.z = sourceNormal[2];
			}
			
			KFbxColor color;
			b = pullT<KFbxLayerElementVertexColor, KFbxColor>(
				mesh, vertexId, controlPointIndex, &KFbxLayer::GetVertexColors, color);
			if(b)
				vert.color = ColorToInt(color.mRed, color.mGreen, color.mBlue, color.mAlpha);

			pullUVs(mesh, vertexId, controlPointIndex, polygonIndex, vertexIndex, vert);

			if(meshInfo.textureIndex == -1)
			{
				// No textures, and I want more than a white 
				// box to show up in the scene, so random colors:
				vert.color = ColorToInt(rand() % 255, rand() % 255, rand() % 255);
				//vert.color = ColorToInt(255, 0, 0);
			}
			vertexMap.push_back(controlPointIndex);
			vertss.push_back(vert);

			vertexId++;
		} 
		addIndices(numVertsThisPolygon, vertss, indicess);
	} 
	addWeights(vertss);
	meshInfo.indices = toArray(indicess);
	meshInfo.numIndices = indicess.size();
	meshInfo.vertices = toArray(vertss);
	meshInfo.numVerts = vertss.size();
	if(isDuplicateMesh(meshInfo))
		meshInfo.destroy();
	else
		geometries.push_back(meshInfo);
}

template <class TVert>
void FbxExtractor<TVert>::pullNodeInfo(KFbxNode* node)
{
	const char* nodeName= node->GetName();
	pullSkeletonInfo(node);
	KFbxMesh* mesh = node->GetMesh();
	if(mesh != NULL)
		pullFbxData(nodeName, mesh);
}

template <class TVert>
void FbxExtractor<TVert>::pullSkinInfo(KFbxMesh* mesh)
{
	KFbxCluster* lCluster;
	for(int i=0;i<mesh->GetDeformerCount(KFbxDeformer::eSkin);i++)
	{
		for(int j=0;j<((FbxSkin *) mesh->GetDeformer(i, FbxDeformer::eSkin))->GetClusterCount();j++)
		{
			lCluster = ((FbxSkin *) mesh->GetDeformer(i, FbxDeformer::eSkin))->GetCluster(j);
			double *weights = lCluster->GetControlPointWeights();
			const char* boneName = lCluster->GetLink()->GetName();
			
			char* boneNameLocation = new char[strlen(boneName)+1];
			strcpy_s(boneNameLocation,strlen(boneName)+1,boneName);

			int *lControlPointIndex=lCluster->GetControlPointIndices();
			for(int x=0;x<lCluster->GetControlPointIndicesCount();x++)
			{
				weightMap.push_back(controlPointWeightMap(lControlPointIndex[x],NULL,weights[x]));
			}
			boneNameLocation = 0;
		}
	}
}

template <class TVert>
bool FbxExtractor<TVert>::pullScene(
	const char* fileName,
	std::vector<GeometryToolData<TVert>>& baseGeometries,
	std::vector<std::string>& textureFileNames,
	std::vector<ushort>& animIndicies,
	std::vector<AnimationKeyFrameSet>& animationKeySet,
	std::vector<skeletonBone>& skele)
{
	KFbxScene* gScene = InitializeSdkManagerAndScene();

	if(!LoadFBXScene(fileName))
		return false;
	
	size_t numNodes = gScene->GetNodeCount();

	for(size_t i = 0; i < numNodes; i++)
	{
		pullNodeInfo(gScene->GetNode(i));

		//extract animatnions
		int numStacks = gScene->GetSrcObjectCount(FBX_TYPE(FbxAnimStack));
		FbxAnimStack* pAnimStack;
		for(int k=0;k<numStacks;k++)
		{
			FbxAnimStack* pAnimStack = FbxCast<FbxAnimStack>(gScene->GetSrcObject(FBX_TYPE(FbxAnimStack), k));
			int numAnimLayers = pAnimStack->GetMemberCount(FBX_TYPE(FbxAnimLayer));
			for(int j=0;j<numAnimLayers;j++)
			{
				FbxAnimLayer* lAnimLayer = pAnimStack->GetMember(FBX_TYPE(FbxAnimLayer), j);
				if(AnimationExists(lAnimLayer,gScene->GetNode(i)))
				{
					AnimationTake(lAnimLayer,gScene->GetNode(i), gScene);
				}
			}
		}
	}
	for(int i=0;i<animKeySet.size();i++)
	{
		animationKeySet.push_back(animKeySet[i]);

	}
	textureFileNames = this->textureFileNames; // :) tool code
	baseGeometries = geometries;
	skele = skeleton;

	this->skeleton.clear();
	this->geometries.clear();
	this->textureFileNames.clear();
	this->vertexMap.clear();
	this->animKeySet.clear();
	this->weightMap.clear();
	DestroySdkObjects();
	return true;
}

template<class TVert>
bool FbxExtractor<TVert>::isDuplicateMesh(const GeometryToolData<TVert>& data) const
{
	for(uint i = 0; i < geometries.size(); i++)
	{
		const GeometryToolData<TVert>& g = geometries[i];
		if(g.numIndices != data.numIndices || g.numVerts != data.numVerts)
			continue;
		for(uint j = 0; j < data.numIndices; j++)
			if(g.indices[i] != data.indices[i])
				goto EndLoop;
		for(uint j = 0; j < data.numVerts; j++)
			if(memcmp(data.vertices + j, g.vertices + j, sizeof(TVert)) != 0)
				goto EndLoop;
		return true;
EndLoop: {}
	}
	return false;
}

template<class TVert>
bool FbxExtractor<TVert>::ExtractAnimation(KFbxScene * pScene,KFbxNode* node)
{
	int numStacks = pScene->GetSrcObjectCount(FBX_TYPE(FbxAnimStack));
	FbxAnimStack* pAnimStack;
	for(int i=0;i<numStacks;i++)
	{
		FbxAnimStack* pAnimStack = FbxCast<FbxAnimStack>(pScene->GetSrcObject(FBX_TYPE(FbxAnimStack), i));
		int numAnimLayers = pAnimStack->GetMemberCount(FBX_TYPE(FbxAnimLayer));
		for(int j=0;j<numAnimLayers;j++)
		{
			FbxAnimLayer* lAnimLayer = pAnimStack->GetMember(FBX_TYPE(FbxAnimLayer), j);
			
			FbxAnimCurve* lAnimCurve = node->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_TRANSFORM);
			FbxTimeSpan interval;
			node->GetAnimationInterval(interval,pAnimStack,j);
			if(lAnimCurve)
				int keyFrameCount = lAnimCurve->KeyGetCount();

			/*for(int n=0;n<keyFrameCount;n++)
			{
				FbxTime startTime, endTime, nextTime;
				startTime = lAnimCurve->KeyGetTime(n);
			}*/
		}
	}
	return true;
}


template<class TVert>
bool FbxExtractor<TVert>::AnimationExists(FbxAnimLayer *pAnimLayer,KFbxNode* pNode)
{
	{

    FbxAnimCurve *pCurveTX = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);

		if (pCurveTX)
		{
			int lKeyCountTX = pCurveTX->KeyGetCount();
			if (lKeyCountTX > 0)
				return true;
		}
	}
	{
    FbxAnimCurve *pCurveTY = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (pCurveTY)
		{
			int lKeyCountTY = pCurveTY->KeyGetCount();
			if (lKeyCountTY > 0)
				return true;
		}
	}
	{
    FbxAnimCurve *pCurveTZ = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (pCurveTZ)
		{
			int lKeyCountTZ = pCurveTZ->KeyGetCount();
			if (lKeyCountTZ > 0)
				return true;
		}     
	}
	/////////// SCALE
	{
    FbxAnimCurve*  pCurveSX = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (pCurveSX)
		{
			int lKeyCountSX = pCurveSX->KeyGetCount();
			if (lKeyCountSX > 0)
				return true;
		}
	}
	{
    FbxAnimCurve*  pCurveSY = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (pCurveSY)
		{
			int lKeyCountSY = pCurveSY->KeyGetCount();
			if (lKeyCountSY > 0)
				return true;
		}
	}
	{
    FbxAnimCurve*  pCurveSZ = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (pCurveSZ)
		{
			int lKeyCountSZ = pCurveSZ->KeyGetCount();
			if (lKeyCountSZ > 0)
				return true;   
		}
	}
	/////////// ROTATE
	{
    FbxAnimCurve* pCurveRX = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (pCurveRX)
		{
			int lKeyCountRX = pCurveRX->KeyGetCount();

			if (lKeyCountRX > 0)
				return true;
		}
	}
	{
    FbxAnimCurve* pCurveRY = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (pCurveRY)
		{
			int lKeyCountRY = pCurveRY->KeyGetCount();

			if (lKeyCountRY > 0)
				return true;
		}
	}
	{
    FbxAnimCurve* pCurveRZ = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (pCurveRZ)
		{
			int lKeyCountRZ = pCurveRZ->KeyGetCount();
			if (lKeyCountRZ > 0)
				return true;
		}   
	}
	return false;
}

template<class TVert>
void FbxExtractor<TVert>::AnimationTake(FbxAnimLayer* pAnimLayer,FbxNode* pNode, KFbxScene * pScene)
{
	animKeySet.push_back(AnimationKeyFrameSet());
	AnimationKeyFrameSet& animationKeySet = animKeySet[animKeySet.size()-1];
	animationKeySet.clear();
	const char * pNodeName = pNode->GetName();
	animKeySet[animKeySet.size()-1].index = skeleton.size();
	//bool bAnimationPresent = AnimationPresent(pAnimLayer, pNode);
	
	FbxTakeInfo* lCurrentTakeInfo = pScene->GetTakeInfo(std::string(pAnimLayer->GetName()).c_str());
	FbxTime tStart, tStop;

	if (lCurrentTakeInfo)
	{
		tStart = lCurrentTakeInfo->mLocalTimeSpan.GetStart();
		tStop = lCurrentTakeInfo->mLocalTimeSpan.GetStop();
	}
	else
	{
		// filled in by animation
		tStart.SetSecondDouble(FLT_MAX);
		tStop.SetSecondDouble(-FLT_MAX);
	} 

	FbxTime::EMode pTimeMode = pScene->GetGlobalSettings().GetTimeMode();
	double frameRate = FbxTime::GetFrameRate(pTimeMode);

	std::set<FbxTime> timeSet;

	FbxAnimCurve* pCurveRX = 0;
	FbxAnimCurve* pCurveRY = 0;
	FbxAnimCurve* pCurveRZ = 0;

	FbxAnimCurve* pCurveSX = 0;
	FbxAnimCurve* pCurveSY = 0;
	FbxAnimCurve* pCurveSZ = 0;

	FbxAnimCurve* pCurveTX = 0;
	FbxAnimCurve* pCurveTY = 0;
	FbxAnimCurve* pCurveTZ = 0;

	std::vector<RotateKey> raw_rx;
	std::vector<RotateKey> raw_ry;
	std::vector<RotateKey> raw_rz;
	///////
	////// TRANS
	{
		pCurveTX = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (pCurveTX)
		{
			int lKeyCountTX = pCurveTX->KeyGetCount();

			for(int lCount = 0; lCount < lKeyCountTX; lCount++)
			{
				FbxTime   lKeyTime = pCurveTX->KeyGetTime(lCount);    
				if (lKeyTime < tStart)
					tStart = lKeyTime;

				if (lKeyTime > tStop)
					tStop = lKeyTime;

				timeSet.insert(lKeyTime);
			}
		}
	}
	{
		pCurveTY = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (pCurveTY)
		{
			int lKeyCountTY = pCurveTY->KeyGetCount();

			for(int lCount = 0; lCount < lKeyCountTY; lCount++)
			{
				FbxTime   lKeyTime = pCurveTY->KeyGetTime(lCount);    
				if (lKeyTime < tStart)
					tStart = lKeyTime;

				if (lKeyTime > tStop)
					tStop = lKeyTime;
				timeSet.insert(lKeyTime);


			}
		}
	}
	{
		pCurveTZ = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (pCurveTZ)
		{
			int lKeyCountTZ = pCurveTZ->KeyGetCount();

			for(int lCount = 0; lCount < lKeyCountTZ; lCount++)
			{
				FbxTime   lKeyTime = pCurveTZ->KeyGetTime(lCount);   
				if (lKeyTime < tStart)
					tStart = lKeyTime;

				if (lKeyTime > tStop)
					tStop = lKeyTime;

				timeSet.insert(lKeyTime);
			}
		}     
	}
	/////////// SCALE
	{
		pCurveSX = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
		if (pCurveSX)
		{
			int lKeyCountSX = pCurveSX->KeyGetCount();

			for(int lCount = 0; lCount < lKeyCountSX; lCount++)
			{
				FbxTime   lKeyTime = pCurveSX->KeyGetTime(lCount);    
				if (lKeyTime < tStart)
					tStart = lKeyTime;

				if (lKeyTime > tStop)
					tStop = lKeyTime;

				timeSet.insert(lKeyTime);
			}
		}
	}
	{
		pCurveSY = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (pCurveSY)
		{
			int lKeyCountSY = pCurveSY->KeyGetCount();

			for(int lCount = 0; lCount < lKeyCountSY; lCount++)
			{
				FbxTime   lKeyTime = pCurveSY->KeyGetTime(lCount);    
				if (lKeyTime < tStart)
					tStart = lKeyTime;

				if (lKeyTime > tStop)
					tStop = lKeyTime;

				timeSet.insert(lKeyTime);        
			}
		}
	}
	{
		pCurveSZ = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (pCurveSZ)
		{
			int lKeyCountSZ = pCurveSZ->KeyGetCount();

			for(int lCount = 0; lCount < lKeyCountSZ; lCount++)
			{
				FbxTime   lKeyTime = pCurveSZ->KeyGetTime(lCount);    
				if (lKeyTime < tStart)
					tStart = lKeyTime;

				if (lKeyTime > tStop)
					tStop = lKeyTime;

				timeSet.insert(lKeyTime);        
			}     
		}
	}
	/////////// ROTATE
	{
		pCurveRX = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);

		if (pCurveRX)
		{
			int lKeyCountRX = pCurveRX->KeyGetCount();

			for(int lCount = 0; lCount < lKeyCountRX; lCount++)
			{
				FbxTime   lKeyTime = pCurveRX->KeyGetTime(lCount);    
				float lKeyValue = static_cast<float>(pCurveRX->KeyGetValue(lCount));

				if (lKeyTime < tStart)
					tStart = lKeyTime;

				if (lKeyTime > tStop)
					tStop = lKeyTime;

				timeSet.insert(lKeyTime);

				RotateKey rk;
				rk.time = lKeyTime.GetSecondDouble() * frameRate;   
				rk.value = lKeyValue;   
				raw_rx.push_back(rk);

			}
		}
	}
	{
		pCurveRY = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
		if (pCurveRY)
		{
			int lKeyCountRY = pCurveRY->KeyGetCount();

			for(int lCount = 0; lCount < lKeyCountRY; lCount++)
			{
				FbxTime   lKeyTime = pCurveRY->KeyGetTime(lCount);    
				float lKeyValue = static_cast<float>(pCurveRY->KeyGetValue(lCount));

				if (lKeyTime < tStart)
					tStart = lKeyTime;

				if (lKeyTime > tStop)
					tStop = lKeyTime;

				timeSet.insert(lKeyTime);        

				RotateKey rk;
				rk.time = lKeyTime.GetSecondDouble() * frameRate;   
				rk.value = lKeyValue;   
				raw_ry.push_back(rk);
			}
		}
	}
	{
		pCurveRZ = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
		if (pCurveRZ)
		{
			int lKeyCountRZ = pCurveRZ->KeyGetCount();

			for(int lCount = 0; lCount < lKeyCountRZ; lCount++)
			{
				FbxTime lKeyTime = pCurveRZ->KeyGetTime(lCount);    
				float lKeyValue = static_cast<float>(pCurveRZ->KeyGetValue(lCount));

				if (lKeyTime < tStart)
					tStart = lKeyTime;

				if (lKeyTime > tStop)
					tStop = lKeyTime;
				timeSet.insert(lKeyTime);   

				RotateKey rk;
				rk.time = lKeyTime.GetSecondDouble() * frameRate;   
				rk.value = lKeyValue;   
				raw_rz.push_back(rk);
			}  
		}   
	}
 double timeStartSecondsTimesRate = tStart.GetSecondDouble() * frameRate;
  double timeStopSecondsTimesRate = tStop.GetSecondDouble() * frameRate;  

 /* lpAnimationController * m_pAnimController->m_frameRate = frameRate;
  if (timeStartSecondsTimesRate < m_pAnimController->m_timeStartSecondsTimesRate)
    m_pAnimController->m_timeStartSecondsTimesRate = timeStartSecondsTimesRate;

  if (timeStopSecondsTimesRate > m_pAnimController->m_timeStopSecondsTimesRate)
    m_pAnimController->m_timeStopSecondsTimesRate = timeStopSecondsTimesRate;*/

  /////////////////////////
  timeSet.insert(tStart);
  timeSet.insert(tStop);

  std::set<FbxTime>::iterator it;

  const char * name = pNode->GetName();




  //Check for rotational overflow.
  bool bFirst = true;
  FbxVector4 lastRot;

	FbxVector4 localT, localR, localS;

  
  localT  = pNode->LclTranslation.Get();
  localR  = pNode->LclRotation.Get();
  localS  = pNode->LclScaling.Get();



	//glm::vec4 localQuat;
	//FBXQuaternionFromEuler( localQuat, localR);

  bFirst = true;
  glm::vec4 lastQuat;


	for (it = timeSet.begin(); it != timeSet.end(); it++)
	{
		const FbxTime &pTime = *it;

    FbxAMatrix localMatrix = pNode->GetScene()->GetEvaluator()->GetNodeLocalTransform(pNode, pTime);;
    FbxVector4 scale = localMatrix.GetS();

		TimeKey scaleKey;
		scaleKey.time = pTime.GetSecondDouble() * frameRate;   


		scaleKey.value.x = scale[0];
		scaleKey.value.y = scale[1];
		scaleKey.value.z = scale[2];

    //is_valid(scaleKey.value);
		animationKeySet.scaleKeys.push_back(scaleKey);

    FbxVector4 trans = localMatrix.GetT();
    TimeKey transKey;

		transKey.time = pTime.GetSecondDouble() * frameRate;   


    if (_isnan(trans[0]) || !_finite(trans[0]))
    {
      FbxAMatrix localMatrix2 = pNode->GetScene()->GetEvaluator()->GetNodeLocalTransform(pNode, pTime);

      //pCurveTX = pNode->LclTranslation.GetCurve<FbxAnimCurve>(pAnimLayer, KFCURVENODE_T_X);
      class  TV
      {
      public:
        double time, value;
      };
      std::vector<TV> values;

      if (pCurveTX)
      {


        //stop(); // need to investivate
#if 0
        int lKeyCountTX = pCurveTX->KeyGetCount();

        for(int lCount = 0; lCount < lKeyCountTX; lCount++)
        {
          TV tv;
          FbxTime   lKeyTime = pCurveTX->KeyGetTime(lCount);    
          tv.time = lKeyTime.GetSecondDouble();
          tv.value = pCurveTX->KeyGetValue(lCount);
          values.push_back(tv);
        }
#endif
      }
      else
        trans[0] = localT[0];
    }
    if (_isnan(trans[1]) || !_finite(trans[1]))
      trans[1] = localT[1];
      
    if (_isnan(trans[2]) || !_finite(trans[2]))
      trans[2] = localT[2];

		transKey.value.x = trans[0];
		transKey.value.y = trans[1];
		transKey.value.z = trans[2];
    

    //is_valid(transKey.value);

		animationKeySet.transKeys.push_back(transKey);
	

    FbxQuaternion localQ = localMatrix.GetQ();
		FbxVector4 localRot = localMatrix.GetR();




		TimeKey rotKey;
		rotKey.time = pTime.GetSecondDouble() * frameRate;   

		rotKey.value.x = localRot[0];
		rotKey.value.y = localRot[1];
		rotKey.value.z = localRot[2];


    //is_valid(rotKey.value);

		animationKeySet.rotKeys.push_back(rotKey);



		TimeKeyFrame qkey;
    qkey.time = pTime.GetSecondDouble() * frameRate; 

    glm::vec4 quat;

    quat.x = localQ[0];
    quat.y = localQ[1];
    quat.z = localQ[2];
    quat.w = localQ[3];

    if (!bFirst)
		{

        // shortest path
        if (quatDot(quat, lastQuat) < 0)
        {
          quat.x = -quat.x;
          quat.y = -quat.y;
          quat.z = -quat.z;
          quat.w = -quat.w;
        } 
      
      
      /*
        // take longest path
        if (quatDot(quat, lastQuat) > 0)
        {
          quat.i = -quat.i;
          quat.j = -quat.j;
          quat.k = -quat.k;
          quat.s = -quat.s;
        }
      }*/
		}
		bFirst = false;
		lastQuat = quat;

		qkey.value = quat;


		animationKeySet.quatKeys.push_back(qkey);
	}
	if (animationKeySet.scaleKeys.size() > 0 || animationKeySet.transKeys.size() > 0 || animationKeySet.rotKeys.size() > 0)
	{
		FbxTime pTime = 0;
		FbxAMatrix defaultPosition = pNode->GetScene()->GetEvaluator()->GetNodeLocalTransform(pNode);

		if (animationKeySet.transKeys.size()  == 0)
		{
			// default translation
			TimeKey key;
			key.time = 0;

			FbxVector4 trans = defaultPosition.GetT();

			key.value.x = trans[0];
			key.value.y = trans[1];
			key.value.z = trans[2];

			animationKeySet.transKeys.push_back(key);
		}
		if (animationKeySet.scaleKeys.size() == 0)
		{
			// default scale
			TimeKey key;
			key.time = 0;

			FbxVector4 scale = defaultPosition.GetS();

			key.value.x = scale[0];
			key.value.y = scale[1];
			key.value.z = scale[2];

			animationKeySet.scaleKeys.push_back(key);
		}
		if (animationKeySet.rotKeys.size() == 0)
		{

			TimeKeyFrame qkey;
			qkey.time = 0;

			glm::vec4 quat;

			FbxVector4 rot = defaultPosition.GetR();
      FbxQuaternion localQ = defaultPosition.GetQ();

      //FBXQuaternionFromEuler( quat, rot);
      quat.x = localQ[0];
      quat.y = localQ[1];
      quat.z = localQ[2];
      quat.w = localQ[3];

			qkey.value = quat;

			//qkey.value.i = quat[0];
			//qkey.value.j = quat[1];
			//qkey.value.k = quat[2];
			//qkey.value.s = quat[3];
			animationKeySet.quatKeys.push_back(qkey);    

		// default rotation
			TimeKey key;
			key.time = 0;


			key.value.x = rot[0];
			key.value.y = rot[1];
			key.value.z = rot[2];

			animationKeySet.rotKeys.push_back(key);    
		}
	}
}


template<typename TVert>
void FbxExtractor<TVert>::addWeights(vector<TVert>& verts)
{
	//needs to compare the weights and average them.
	//anything past the 4th smallest one should be thrown out
	//we then only store the top 3 and assume the smallest is w4 = 1 - (w1 + w2 + w3)

	vector<vertexWeight> weights;
	for(int i=0;i<weightMap.size();i++)
	{
		for(int j=0;j<skeleton.size();j++)
		{
			if(strcmp(skeleton[j].boneName,weightMap[i].boneName) ==0)
			{
				weightMap[i].boneID = j+1;
			}
		}
	}
	for(int i=0;i<vertexMap.size();i++)
	{
		vertexWeights.push_back(vertexWeightData());
		for(int j=0;j<weightMap.size();j++)
		{
			if(vertexMap[i] == weightMap[j].index)
			{
				weights.push_back(vertexWeight(weightMap[j].index,weightMap[j].boneID,weightMap[j].weight));
			}
		}
		if(weights.size() > 4)
		{
			for(uint j=0;j<weights.size();j++)
			{
				int count=0;
				for(uint x=0;x<weights.size();x++)
				{					
					if(weights[j].weight == weights[x].weight && i != j)
					{
						//adjust the value of one weight on the off chance 2 are the same
						weights[x].weight += 0.000000000000000000000000000001f;
						weights[j].weight -= 0.000000000000000000000000000001f;
					}
					if(weights[j].weight < weights[x].weight)
					{
						count++;
					}
				}
				if (count < 4)
				{
					//set up the indicies of skeleton weights
					vertexWeights[vertexWeights.size()-1].boneID |= weights[j].boneID<<(count*8);
					vertexWeights[vertexWeights.size()-1].weight[count] = weights[j].weight;
				}
			}
		}
		else
		{
			//add weights
			for(uint j=0;j<weights.size();j++)
			{				
				int count=0;
				for(uint x=0;x<weights.size();x++)
				{
					if(weights[j].weight == weights[x].weight && i != j)
					{
						//adjust the value of one weight on the off chance 2 are the same
						weights[x].weight += 0.000000000000000000000000000001f;
						weights[j].weight -= 0.000000000000000000000000000001f;
					}
					if(weights[j].weight < weights[x].weight)
					{
						count++;
					}
					vertexWeights[vertexWeights.size()-1].boneID |= weights[j].boneID<<(count*8);
					vertexWeights[vertexWeights.size()-1].weight[count] = weights[j].weight;
				}
				if(weights.size() == 1)
				{
					vertexWeights[vertexWeights.size()-1].boneID |= weights[j].boneID;
					vertexWeights[vertexWeights.size()-1].weight[count] = weights[j].weight;
				}
			}
			weights.clear();
		}
	}
	for(int i=0;i<verts.size();i++)
	{
		verts[i].skeletonIDs = vertexWeights[i].boneID;
		verts[i].w1 = vertexWeights[i].weight[0];
		verts[i].w2 = vertexWeights[i].weight[1];
		verts[i].w3 = vertexWeights[i].weight[2];
	}
	vertexWeights.clear();
	weightMap.clear();
}

template<class TVert>
void FbxExtractor<TVert>::pullSkeletonInfo(KFbxNode* node)
{
	KFbxSkeleton* lSkeleton = (KFbxSkeleton*) node->GetNodeAttribute();
	if(lSkeleton)
	{
		if(lSkeleton->GetSkeletonType() == KFbxSkeleton::eRoot || lSkeleton->GetSkeletonType() == KFbxSkeleton::eLimbNode || lSkeleton->GetSkeletonType() == KFbxSkeleton::eEffector)
		{
			//eEffector is the root node
			const char* name = node->GetName();
			skeleton.push_back(skeletonBone());
			strcpy(skeleton[skeleton.size()-1].boneName,name);
			skeleton[skeleton.size()-1].boneName[15] = '\0';
			assert(skeleton.size()<255);
			//animKeySet.push_back(AnimationKeyFrameSet());
			//TODO::
			//need to store the parentIDs once I find them.
			// the current id of each bone is the vector index+1
		}
	}
}


#endif
