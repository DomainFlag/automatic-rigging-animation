#include <iostream>
#include <fbxsdk.h>

#include "../mesh.h"

using namespace std;

#ifdef IOS_REF
	#undef  IOS_REF
	#define IOS_REF (*(pManager->GetIOSettings()))
#endif

void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
    //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    pManager = FbxManager::Create();
    if( !pManager )
    {
        FBXSDK_printf("Error: Unable to create FBX Manager!\n");
        exit(1);
    }
	else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());

    //Create an FBX scene. This object holds most objects imported/exported from/to files.
    pScene = FbxScene::Create(pManager, "My Scene");
	if( !pScene )
    {
        FBXSDK_printf("Error: Unable to create FBX scene!\n");
        exit(1);
    }
}

void DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
{
    //Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
    if( pManager ) pManager->Destroy();
	if( pExitStatus ) FBXSDK_printf("Program Success!\n");
}

bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
{
    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor,  lSDKMinor,  lSDKRevision;
    //int lFileFormat = -1;
    int lAnimStackCount;
    bool lStatus;
    char lPassword[1024];

    // Get the file version number generate by the FBX SDK.
    FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

    // Create an importer.
    FbxImporter * lImporter = FbxImporter::Create(pManager,"");

    // Initialize the importer by providing a filename.
    const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

    if( !lImportStatus )
    {
        FbxString error = lImporter->GetStatus().GetErrorString();
        FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
        FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

        if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
            FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
        }

        return false;
    }

    FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

    if (lImporter->IsFBX())
    {
        FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

        // From this point, it is possible to access animation stack information without
        // the expense of loading the entire file.

        FBXSDK_printf("Animation Stack Information\n");

        lAnimStackCount = lImporter->GetAnimStackCount();

        FBXSDK_printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
        FBXSDK_printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
        FBXSDK_printf("\n");

        for(int i = 0; i < lAnimStackCount; i++)
        {
            FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

            FBXSDK_printf("    Animation Stack %d\n", i);
            FBXSDK_printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
            FBXSDK_printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

            // Change the value of the import name if the animation stack should be imported 
            // under a different name.
            FBXSDK_printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

            // Set the value of the import state to false if the animation stack should be not
            // be imported. 
            FBXSDK_printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
            FBXSDK_printf("\n");
        }

        // Set the import states. By default, the import states are always set to 
        // true. The code below shows how to change these states.
        IOS_REF.SetBoolProp(IMP_FBX_MATERIAL,        true);
        IOS_REF.SetBoolProp(IMP_FBX_TEXTURE,         true);
        IOS_REF.SetBoolProp(IMP_FBX_LINK,            true);
        IOS_REF.SetBoolProp(IMP_FBX_SHAPE,           true);
        IOS_REF.SetBoolProp(IMP_FBX_GOBO,            true);
        IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    }

    // Import the scene.
    lStatus = lImporter->Import(pScene);
	if (lStatus == true)
	{
		// Check the scene integrity!
		FbxStatus status;
		FbxArray< FbxString*> details;
		FbxSceneCheckUtility sceneCheck(FbxCast<FbxScene>(pScene), &status, &details);
		lStatus = sceneCheck.Validate(FbxSceneCheckUtility::eCkeckData);
		bool lNotify = (!lStatus && details.GetCount() > 0) || (lImporter->GetStatus().GetCode() != FbxStatus::eSuccess);
		if (lNotify)
		{
			FBXSDK_printf("\n");
			FBXSDK_printf("********************************************************************************\n");
			if (details.GetCount())
			{
				FBXSDK_printf("Scene integrity verification failed with the following errors:\n");
				for (int i = 0; i < details.GetCount(); i++)
					FBXSDK_printf("   %s\n", details[i]->Buffer());
				
				FbxArrayDelete<FbxString*>(details);
			}

			if (lImporter->GetStatus().GetCode() != FbxStatus::eSuccess)
			{
				FBXSDK_printf("\n");
				FBXSDK_printf("WARNING:\n");
				FBXSDK_printf("   The importer was able to read the file but with errors.\n");
				FBXSDK_printf("   Loaded scene may be incomplete.\n\n");
				FBXSDK_printf("   Last error message:'%s'\n", lImporter->GetStatus().GetErrorString());
			}
			FBXSDK_printf("********************************************************************************\n");
			FBXSDK_printf("\n");
		}
	}

    if(lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
    {
        FBXSDK_printf("Please enter password: ");

        lPassword[0] = '\0';

        FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
        scanf("%s", lPassword);
        FBXSDK_CRT_SECURE_NO_WARNING_END

        FbxString lString(lPassword);

        IOS_REF.SetStringProp(IMP_FBX_PASSWORD,      lString);
        IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

        lStatus = lImporter->Import(pScene);

        if(lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
        {
            FBXSDK_printf("\nPassword is wrong, import aborted.\n");
        }
    }

    // Destroy the importer.
    lImporter->Destroy();

    return lStatus;
}

FbxVector4 getMeshNormal(const FbxGeometryElementNormal * normalElement, int polyIndex, int posIndex) {
    if (normalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
        if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
            return normalElement->GetDirectArray().GetAt(posIndex);
        int i = normalElement->GetIndexArray().GetAt(posIndex);
        return normalElement->GetDirectArray().GetAt(i);
    }
    else if (normalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
        if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
            return normalElement->GetDirectArray().GetAt(polyIndex);
        int i = normalElement->GetIndexArray().GetAt(polyIndex);
        return normalElement->GetDirectArray().GetAt(i);
    }
    return FbxVector4();
}

FbxVector2 getMeshUV(const FbxGeometryElementUV * uvElement, int polyIndex, int posIndex) {
    if (uvElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
        if (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect)
            return uvElement->GetDirectArray().GetAt(posIndex);

        int i = uvElement->GetIndexArray().GetAt(posIndex);

        return uvElement->GetDirectArray().GetAt(i);
    } else if (uvElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
        if (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect)
            return uvElement->GetDirectArray().GetAt(polyIndex);

        int i = uvElement->GetIndexArray().GetAt(polyIndex);

        return uvElement->GetDirectArray().GetAt(i);
    }

    return FbxVector2();
}

int getMeshMaterialId(Mesh & base, const FbxNode * root, const FbxMesh * mesh, int polygonIndex) {
    for (int g = 0; g < mesh->GetElementMaterialCount(); g++) {
        const FbxGeometryElementMaterial * materialElement = mesh->GetElementMaterial(g);

        int id = materialElement->GetIndexArray().GetAt(polygonIndex);
        if (id != -1) {
            FbxSurfaceMaterial * material = (FbxSurfaceMaterial*) root->GetSrcObject<FbxSurfaceMaterial>(id);

            auto target = base.materials.find(material->GetName());
            if (target != base.materials.end()) {
                return target->second.index;
            }
        } 
    }

    return -1;
}

FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial * pMaterial, const char * pPropertyName, const char * pFactorPropertyName, int & pTextureName) {
    FbxDouble3 lResult(0, 0, 0);
    const FbxProperty prop = pMaterial->FindProperty(pPropertyName);
    const FbxProperty lFactorProperty = pMaterial->FindProperty(pFactorPropertyName);
    if (prop.IsValid() && lFactorProperty.IsValid())
    {
        lResult = prop.Get<FbxDouble3>();
        double lFactor = lFactorProperty.Get<FbxDouble>();
        if (lFactor != 1)
        {
            lResult[0] *= lFactor;
            lResult[1] *= lFactor;
            lResult[2] *= lFactor;
        }
    }

    auto pr = pMaterial->GetFirstProperty();
    while(pr.IsValid()) {
        pr = pMaterial->GetNextProperty(pr);

        const int textureCount = pr.GetSrcObjectCount<FbxLayeredTexture>();
        if (textureCount > 0) {
            const FbxLayeredTexture * texture = pr.GetSrcObject<FbxLayeredTexture>();
            if (texture != nullptr) {
                cout << texture->GetName() << endl;
            }
        } else {
            const FbxTexture * texture = pr.GetSrcObject<FbxTexture>();
            if (texture != nullptr) {
                cout << texture->GetName() << endl;
            }
        }
    }

    return lResult;
}

void resolveTexture(FbxSurfaceMaterial * material, FbxFileTexture * texture, Mesh & base, const string &hint = "") {
    string path = texture ? texture->GetFileName() : material->GetName();
    if (!hint.empty()) {
        char arr[256];
        sprintf(arr, hint.c_str(), path.substr(path.find_last_of("/\\") + 1).c_str());

        path = string(arr);
    }

    Material mat(path, base.materials.size());
    base.materials.insert({material->GetName(), mat});
}

void resolveSkeleton(FbxNode * root) {
    FbxSkeleton * skeleton = (FbxSkeleton*) root->GetNodeAttribute();

    cout <<  "Skeleton Name: " << (char *) skeleton->GetName() << endl;
    int nbMetaData = skeleton->GetSrcObjectCount<FbxObjectMetaData>();
    if (nbMetaData > 0)
        cout << "    MetaData connections " << endl;

    for (int i = 0; i < nbMetaData; i++) {
        FbxObjectMetaData* metaData = skeleton->GetSrcObject<FbxObjectMetaData>(i);
        cout << "        Name: " << (char*) metaData->GetName() << endl;
    }
}
 
void resolveNode(FbxNode * root, Mesh & base, const string &hint) {
    if (root) {    
        int childCount = root->GetChildCount();
        for (int i = 0; i < childCount; i++) {
            FbxNode * node = root->GetChild(i);
            FbxMesh * mesh = node->GetMesh();

            if (node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
                resolveSkeleton(node);
            }

            if (mesh) {
                int controlPointsCount = mesh->GetControlPointsCount();

                cout << i << ". Control points count - " << controlPointsCount << endl;
                cout << i << ". Polygon count - " << mesh->GetPolygonCount() << endl;
                cout << i << ". Material count - " << mesh->GetElementMaterialCount() << endl;
                
                FbxVector4 * controlPoints = mesh->GetControlPoints();

                FbxAMatrix matrixGeo;
                matrixGeo.SetIdentity();
                if (node->GetNodeAttribute()) {
                    const FbxVector4 lT = node->GetGeometricTranslation(FbxNode::eSourcePivot);
                    const FbxVector4 lR = node->GetGeometricRotation(FbxNode::eSourcePivot);
                    const FbxVector4 lS = node->GetGeometricScaling(FbxNode::eSourcePivot);

                    matrixGeo.SetT(lT);
                    matrixGeo.SetR(lR);
                    matrixGeo.SetS(lS);
                }

                FbxAMatrix globalMatrix = node->EvaluateLocalTransform();
                FbxAMatrix matrix = globalMatrix * matrixGeo;

                // FBX textures
                int materialCount = node->GetSrcObjectCount<FbxSurfaceMaterial>();
                for (int index = 0; index < materialCount; index++) {
                    FbxSurfaceMaterial* material = (FbxSurfaceMaterial*) node->GetSrcObject<FbxSurfaceMaterial>(index);
                    if (material) {
                        FBXSDK_printf("material name: %s\n", material->GetName());
                        FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);

                        int size = base.materials.size();
                        int layeredTextureCount = prop.GetSrcObjectCount<FbxLayeredTexture>();
                        if (layeredTextureCount > 0) {
                            for (int j = 0; j < layeredTextureCount; j++) {
                                FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(prop.GetSrcObject<FbxLayeredTexture>(j));
                                int count = layered_texture->GetSrcObjectCount<FbxTexture>();

                                for (int k = 0; k < count; k++) {
                                    FbxFileTexture * texture = FbxCast<FbxFileTexture>(layered_texture->GetSrcObject<FbxFileTexture>(k));
                                    FBXSDK_printf("texture name %d: %s\n", k, texture->GetFileName());

                                    resolveTexture(material, texture, base, hint);
                                }
                            }
                        } else {
                            int textureCount = prop.GetSrcObjectCount<FbxFileTexture>();
                            for (int j = 0; j < textureCount; j++) {
                                FbxFileTexture * texture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxFileTexture>(j));
                                FBXSDK_printf("texture name: %s\n", texture->GetFileName());

                                resolveTexture(material, texture, base, hint);
                            }
                        }

                        if (size == base.materials.size()) {
                            resolveTexture(material, nullptr, base, hint);
                        }
                    }
                }

                // FBX control points
                const int offset = base.vertices.size();
                base.vertices.resize(base.vertices.size() + mesh->GetControlPointsCount());
                for (int index = 0; index < mesh->GetControlPointsCount(); index++) {
                    FbxVector4 vertix = matrix.MultT(controlPoints[index]);

                    base.vertices[offset + index].pos[0] = vertix[0];
                    base.vertices[offset + index].pos[1] = vertix[1];
                    base.vertices[offset + index].pos[2] = vertix[2];
                }

                // FBX skin
                int count = mesh->GetDeformerCount(FbxDeformer::eSkin);
                if (count > 0) {
                    cout << "Deformer count: " << count << endl;
                    for (int g = 0; g < count; g++) {
                        int clusterCount = ((FbxSkin *) mesh->GetDeformer(g, FbxDeformer::eSkin))->GetClusterCount();

                        cout << "Cluster count: " << clusterCount << endl;
                        for (int j = 0; j < clusterCount; j++) {
                            cout << "    Cluster " << i << endl;

                            FbxCluster * cluster = ((FbxSkin *) mesh->GetDeformer(g, FbxDeformer::eSkin))->GetCluster(j);

                            const char * modes[] = { "Normalize", "Additive", "Total" };

                            if (!cluster->GetLink())
                                continue;

                            string name = cluster->GetLink()->GetName();
                            cout << "    Mode: " << modes[cluster->GetLinkMode()] << " - " << name << endl;

                            FbxAMatrix & matrix = cluster->GetLink()->GetChild(0)->EvaluateGlobalTransform();
                            FbxVector4 transformation = matrix.GetT();
                            if (false) {
                                transformation /= matrix.GetS();
                            }

                            base.joints.insert({ name, Pinocchio::Vector3(transformation[0], transformation[1], transformation[2]) });

                            int clusterControlPointsCount = cluster->GetControlPointIndicesCount();
                            for (int k = 0; k < clusterControlPointsCount; k++) {
                                int index = cluster->GetControlPointIndices()[k];
                                if (index >= controlPointsCount)
                                    continue;

                                double weight = cluster->GetControlPointWeights()[k];
                                if (weight == 0.0) {
                                    continue;
                                }

                                FbxNode* parent = cluster->GetLink();
                                if (parent) {
                                    base.vertices[index].weights.insert({ parent->GetName(), weight });
                                } else {
                                    base.vertices[index].weights.insert({ "hips", weight });
                                }
                            }
                        }
                    }
                }

                // FBX edges
                const FbxGeometryElementUV * geometryUV = mesh->GetElementUV();
                const int offsetEdges = base.edges.size();
                base.edges.resize(base.edges.size() + mesh->GetPolygonCount() * 3);
                for (int index = 0; index < mesh->GetPolygonCount(); index++) {
                    const int polygonSize = mesh->GetPolygonSize(index);

                    if (polygonSize == 4) {
                        cout << "Illegal polygon, expected triangle" << endl;

                        exit(-1);
                    } 

                    for (int polyVertIndex = 0; polyVertIndex < polygonSize; polyVertIndex++) {
                        int vertIndex = mesh->GetPolygonVertex(index, polyVertIndex);
                        if (geometryUV) {
                            FbxVector2 tex = getMeshUV(geometryUV, index, polyVertIndex);
                            int materialId = getMeshMaterialId(base, node, mesh, index);

                            base.vertices[offset + vertIndex].texture = Pinocchio::Vector3(tex[0], 1.0 - tex[1], materialId);
                        }

                        base.edges[offsetEdges + index * 3 + polyVertIndex].vertex = offset + vertIndex;
                    }
                }
            }

            resolveNode(node, base, hint);
        }
    }
}

void parseFbx(Mesh & base, const string &file, const string &hint) {
    FbxManager * manager = NULL;
    FbxScene * scene = NULL;

    // Prepare the FBX SDK.
    InitializeSdkObjects(manager, scene);

    // Load the scene.
    bool result = LoadScene(manager, scene, file.c_str());

    FbxGeometryConverter converter(manager);
    converter.Triangulate(scene, true);

    if(result == false) {
        FBXSDK_printf("\n\nAn error occurred while loading the scene...");
    } else {
        FbxNode * root = scene->GetRootNode();

        cout << "Geometry count: " << scene->GetGeometryCount() << endl;
        cout << "Material count: " << scene->GetMaterialCount() << endl;
        
        resolveNode(root, base, hint);
    }
}