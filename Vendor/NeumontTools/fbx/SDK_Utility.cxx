/**************************************************************************************

 Copyright (C) 2001 - 2009 Autodesk, Inc. and/or its licensors.
 All Rights Reserved.

 The coded instructions, statements, computer programs, and/or related material 
 (collectively the "Data") in these files contain unpublished information 
 proprietary to Autodesk, Inc. and/or its licensors, which is protected by 
 Canada and United States of America federal copyright law and by international 
 treaties. 
 
 The Data may not be disclosed or distributed to third parties, in whole or in
 part, without the prior written consent of Autodesk, Inc. ("Autodesk").

 THE DATA IS PROVIDED "AS IS" AND WITHOUT WARRANTY.
 ALL WARRANTIES ARE EXPRESSLY EXCLUDED AND DISCLAIMED. AUTODESK MAKES NO
 WARRANTY OF ANY KIND WITH RESPECT TO THE DATA, EXPRESS, IMPLIED OR ARISING
 BY CUSTOM OR TRADE USAGE, AND DISCLAIMS ANY IMPLIED WARRANTIES OF TITLE, 
 NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE OR USE. 
 WITHOUT LIMITING THE FOREGOING, AUTODESK DOES NOT WARRANT THAT THE OPERATION
 OF THE DATA WILL BE UNINTERRUPTED OR ERROR FREE. 
 
 IN NO EVENT SHALL AUTODESK, ITS AFFILIATES, PARENT COMPANIES, LICENSORS
 OR SUPPLIERS ("AUTODESK GROUP") BE LIABLE FOR ANY LOSSES, DAMAGES OR EXPENSES
 OF ANY KIND (INCLUDING WITHOUT LIMITATION PUNITIVE OR MULTIPLE DAMAGES OR OTHER
 SPECIAL, DIRECT, INDIRECT, EXEMPLARY, INCIDENTAL, LOSS OF PROFITS, REVENUE
 OR DATA, COST OF COVER OR CONSEQUENTIAL LOSSES OR DAMAGES OF ANY KIND),
 HOWEVER CAUSED, AND REGARDLESS OF THE THEORY OF LIABILITY, WHETHER DERIVED
 FROM CONTRACT, TORT (INCLUDING, BUT NOT LIMITED TO, NEGLIGENCE), OR OTHERWISE,
 ARISING OUT OF OR RELATING TO THE DATA OR ITS USE OR ANY OTHER PERFORMANCE,
 WHETHER OR NOT AUTODESK HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS
 OR DAMAGE. 

**************************************************************************************/
#include <fstream>
using std::ofstream;
#include "SDK_Utility.h"

#ifdef IOS_REF
	#undef  IOS_REF
	#define IOS_REF (*(pSdkManager->GetIOSettings()))
#endif

namespace {
	// declare global
	KFbxSdkManager* gSdkManager = NULL;
	KFbxScene*      gScene      = NULL;
}
bool LoadFBXScene(
                  const char *pFbxFilePath  
                  )
{
    // Load the scene.
    if( LoadScene(gSdkManager, gScene, pFbxFilePath ) == false ) return false;

    return true;
}

// Creates an instance of the SDK manager
// and use the SDK manager to create a new scene
KFbxScene* InitializeSdkManagerAndScene()
{
    // Create the FBX SDK memory manager object.
    // The SDK Manager allocates and frees memory
    // for almost all the classes in the SDK.
    gSdkManager = KFbxSdkManager::Create();

	// create an IOSettings object
	KFbxIOSettings * ios = KFbxIOSettings::Create(gSdkManager, IOSROOT );
	gSdkManager->SetIOSettings(ios);

	gScene = KFbxScene::Create(gSdkManager,"");
	return gScene;
}

// to destroy an instance of the SDK manager
void DestroySdkObjects()
{
    // Delete the FBX SDK manager. All the objects that have been allocated 
    // using the FBX SDK manager and that haven't been explicitly destroyed 
    // are automatically destroyed at the same time.
    if (gSdkManager) gSdkManager->Destroy();
}

// to get the filters for the <Open file> dialog (description + file extention)
const char *GetReaderOFNFilters()
{   
    int nbReaders = gSdkManager->GetIOPluginRegistry()->GetReaderFormatCount();;

    KString s;
    int i = 0;

    for(i=0; i < nbReaders; i++)
    {
        s += gSdkManager->GetIOPluginRegistry()->GetReaderFormatDescription(i);
        s += "|*.";
        s += gSdkManager->GetIOPluginRegistry()->GetReaderFormatExtension(i);
        s += "|";
    }

    // replace | by \0
    int nbChar   = int(strlen(s.Buffer())) + 1;
    char *filter = new char[ nbChar ];
    memset(filter, 0, nbChar);

    strcpy(filter, s.Buffer());

    for(i=0; i < int(strlen(s.Buffer())); i++)
    {
        if(filter[i] == '|')
        {
            filter[i] = 0;
        }
    }

    // the caller must delete this allocated memory
    return filter;
}

// to read a file using an FBXSDK reader
bool LoadScene(
               KFbxSdkManager       *pSdkManager, 
               KFbxScene            *pScene, 
               const char           *pFbxFilePath
               )
{
    bool lStatus;

    // Create an importer.
    KFbxImporter* lImporter = KFbxImporter::Create(pSdkManager,"");

    // Initialize the importer by providing a filename.
    bool lImportStatus = lImporter->Initialize(pFbxFilePath, -1, pSdkManager->GetIOSettings() );

    if( !lImportStatus )
    {
        // Destroy the importer
        lImporter->Destroy();
        return false;
    }

    if (lImporter->IsFBX())
    {
        // Set the import states. By default, the import states are always set to 
        // true. The code below shows how to change these states.
        IOS_REF.SetBoolProp(IMP_FBX_MATERIAL,        true);
        IOS_REF.SetBoolProp(IMP_FBX_TEXTURE,         true);
        IOS_REF.SetBoolProp(IMP_FBX_LINK,            true);
        IOS_REF.SetBoolProp(IMP_FBX_SHAPE,           true);
        IOS_REF.SetBoolProp(IMP_FBX_GOBO,            true);
        IOS_REF.SetBoolProp(IMP_FBX_ANIMATION,       true);
        IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    }

    // Import the scene
    lStatus = lImporter->Import(pScene);

    // Destroy the importer
    lImporter->Destroy();

    return lStatus;
}

// to get the root node
const KFbxNode* GetRootNode()
{
    return gScene->GetRootNode();
}

// to get the root node name
const char * GetRootNodeName()
{
    return GetRootNode()->GetName();
}

// to get a string from the node name and attribute type
KString GetNodeNameAndAttributeTypeName(const KFbxNode* pNode)
{
    KString s = pNode->GetName();
	char buf[20];
	KFbxMesh* mesh = const_cast<KFbxNode*>(pNode)->GetMesh();

    KFbxNodeAttribute::EType lAttributeType;

    if(pNode->GetNodeAttribute() == NULL)
    {
        s += " (No node attribute type)";
    }
    else
    {
        lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

        switch (lAttributeType)
        {
        case KFbxNodeAttribute::eMarker:                s += " (Marker)";               break;
        case KFbxNodeAttribute::eSkeleton:              s += " (Skeleton)";             break;
        case KFbxNodeAttribute::eMesh:                  s += " (Mesh)";                 break;
        case KFbxNodeAttribute::eCamera:                s += " (Camera)";               break;
        case KFbxNodeAttribute::eLight:                 s += " (Light)";                break;
        case KFbxNodeAttribute::eBoundary:              s += " (Boundary)";             break;
        case KFbxNodeAttribute::eOpticalMarker:        s += " (Optical marker)";       break;
        case KFbxNodeAttribute::eOpticalReference:     s += " (Optical reference)";    break;
        case KFbxNodeAttribute::eCameraSwitcher:       s += " (Camera switcher)";      break;
        case KFbxNodeAttribute::eNull:                  s += " (Null)";                 break;
        case KFbxNodeAttribute::ePatch:                 s += " (Patch)";                break;
        case KFbxNodeAttribute::eNurbs:                  s += " (NURB)";                 break;
        case KFbxNodeAttribute::eNurbsSurface:         s += " (Nurbs surface)";        break;
        case KFbxNodeAttribute::eNurbsCurve:           s += " (NURBS curve)";          break;
        case KFbxNodeAttribute::eTrimNurbsSurface:    s += " (Trim nurbs surface)";   break;
        case KFbxNodeAttribute::eUnknown:          s += " (Unidentified)";         break;
        }   
    }

    return s;
}

// to get a string from the node default translation values
KString GetDefaultTranslationInfo(
                                  const KFbxNode* pNode
                                  )
{
    KFbxVector4 v4;
    v4 = ((KFbxNode*)pNode)->LclTranslation.Get();

    //return KString("Translation (X,Y,Z): ") + KString(v4.GetAt(0)) + ", " + KString(v4.GetAt(1)) + ", " + KString(v4.GetAt(2));
	return KString();
}

// to get a string from the node visibility value
KString GetNodeVisibility(
                          const KFbxNode* pNode
                          )
{
    return KString("Visibility: ") + (pNode->GetVisibility() ? "Yes":"No");
}
