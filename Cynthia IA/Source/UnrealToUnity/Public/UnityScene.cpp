

#include "UnityScene.h"

#if WITH_EDITOR

Component::Component()
{
	ID = GetUniqueID();
}
std::string Component::GetSceneData()
{
	return "";
}
uint64 Component::GetUniqueID()
{
	uint64 ID = 1030971880 + ( Counter++);
	return ID;
}

uint64 Component::Counter = 0;

TransformComponent::TransformComponent()
{
	Type = CT_TRANSFORM;

	Reset();
}	
void TransformComponent::Reset()
{
	memset( LocalPosition, 0, sizeof( LocalPosition ) );
	memset( LocalRotation, 0, sizeof( LocalRotation ) );
	memset( LocalScale, 0, sizeof( LocalScale ) );

	LocalRotation[ 3 ] = 1.0f;
	for( int i = 0; i < 3; i++ )
		LocalScale[ i ] = 1.0f;
}
std::string TransformComponent::GetChildrenText()
{	
	if( Owner->Children.size() == 0 )
		return "[]\n";
	else
	{
		std::string Ret = "\n";
		for( int i = 0; i < Owner->Children.size(); i++ )
		{
			auto Child = Owner->Children[ i ];
			TransformComponent* TC = ( TransformComponent*)Child->GetComponent( ComponentType::CT_TRANSFORM );
			char Line[ 256 ];
			//uint64_t ID = 0;
			sprintf_s( Line, "  - {fileID: %lld}\n", TC->ID );
			Ret += Line;
		}

		return Ret;
	}
}
std::string TransformComponent::GetSceneData()
{
	uint64 ParentID = 0;
	if( Owner->Parent )
	{
		TransformComponent * TC = ( TransformComponent*) Owner->Parent->GetComponent( CT_TRANSFORM );
		
		ParentID = TC->ID;
	}
	std::string ChildrenText = GetChildrenText();
	int Size = 1024 + ChildrenText.length();
	char* Text = new char[ Size ];
	sprintf_s( Text, Size,
			   "--- !u!4 &%lld\n"
			   "Transform:\n"
			   "  m_ObjectHideFlags: 0\n"
			   "  m_CorrespondingSourceObject: {fileID: 0}\n"
			   "  m_PrefabInstance: {fileID: 0}\n"
			   "  m_PrefabAsset: {fileID: 0}\n"
			   "  m_GameObject: {fileID: %lld}\n"
			   "  m_LocalRotation: {x: %f, y: %f, z: %f, w: %f}\n"
			   "  m_LocalPosition: {x: %f, y: %f, z: %f}\n"
			   "  m_LocalScale: {x: %f, y: %f, z: %f}\n"
			   "  m_Children: %s"
			   "  m_Father: {fileID: %lld}\n"
			   "  m_RootOrder: 0\n"
			   "  m_LocalEulerAnglesHint: {x: 0, y: 0, z: 0}\n"
			   ,
			   this->ID,
			   Owner->ID,//Gameobject
			   LocalRotation[ 0 ], LocalRotation[ 1 ], LocalRotation[ 2 ], LocalRotation[ 3 ],//Rotation
			   LocalPosition[ 0 ], LocalPosition[ 1 ], LocalPosition[ 2 ],//Position
			   LocalScale[ 0 ], LocalScale[ 1 ], LocalScale[ 2 ],//Scale
			   ChildrenText.c_str(),
			   ParentID//father
			   );
	
	std::string RetStr = Text;

	return RetStr;
}


std::string MeshRenderer::GetSceneData()
{
	std::string MaterialText;
	char Line[ 512 ];

	if( Materials.size() > 0 )
	{
		MeshFilter* MF = (MeshFilter*)Owner->GetComponent( ComponentType::CT_MESHFILTER );
		
		for( int i = 0; i < Materials.size(); i++ )
		{
			int fileIDBase = 2100000;
			UnityMaterial* Mat = Materials[ i ];
			if( MF )
			{
				if( i < MF->Mesh->Sections.size() )
				{
					Mat = Materials[ MF->Mesh->Sections[ i ]->MaterialIndex ];
				}
				else//Don't allow more materials as the last submesh will be rendered with them and will look wrong
					break;
			}
			std::string MatGUID = "0000000000000000f000000000000000";
			if( Mat )
				MatGUID = Mat->GUID;
			sprintf_s( Line, "  - {fileID: %d, guid: %s, type: 2}\n", fileIDBase, MatGUID.c_str() );
			MaterialText += Line;
		}
	}
	else
	{
		sprintf_s( Line, "  - {fileID: 10303, guid: 0000000000000000f000000000000000, type: 0}\n" );
		MaterialText += Line;
	}

	int TextLen = 2048 + MaterialText.length();
	char* Text = new char[ TextLen ];

	sprintf_s( Text, TextLen,
	"--- !u!23 &%lld\n"
	"MeshRenderer:\n"
	"  m_ObjectHideFlags: 0\n"
	"  m_CorrespondingSourceObject: {fileID: 0}\n"
	"  m_PrefabInstance: {fileID: 0}\n"
	"  m_PrefabAsset: {fileID: 0}\n"
	"  m_GameObject: {fileID: %lld}\n"
	"  m_Enabled: 1\n"
	"  m_CastShadows: %d\n"
	"  m_ReceiveShadows: 1\n"
	"  m_DynamicOccludee: 1\n"
	"  m_MotionVectors: 1\n"
	"  m_LightProbeUsage: 1\n"
	"  m_ReflectionProbeUsage: 1\n"
	"  m_RayTracingMode: 2\n"
	"  m_RayTraceProcedural: 0\n"
	"  m_RenderingLayerMask: 1\n"
	"  m_RendererPriority: 0\n"
	"  m_Materials:\n"
	"%s"
	"  m_StaticBatchInfo:\n"
	"    firstSubMesh: 0\n"
	"    subMeshCount: 0\n"
	"  m_StaticBatchRoot: {fileID: 0}\n"
	"  m_ProbeAnchor: {fileID: 0}\n"
	"  m_LightProbeVolumeOverride: {fileID: 0}\n"
	"  m_ScaleInLightmap: 1\n"
	"  m_ReceiveGI: 1\n"
	"  m_PreserveUVs: 1\n"
	"  m_IgnoreNormalsForChartDetection: 0\n"
	"  m_ImportantGI: 0\n"
	"  m_StitchLightmapSeams: 1\n"
	"  m_SelectedEditorRenderState: 3\n"
	"  m_MinimumChartSize: 4\n"
	"  m_AutoUVMaxDistance: 0.5\n"
	"  m_AutoUVMaxAngle: 89\n"
	"  m_LightmapParameters: {fileID: 0}\n"
	"  m_SortingLayerID: 0\n"
	"  m_SortingLayer: 0\n"
	"  m_SortingOrder: 0\n"
	"  m_AdditionalVertexStreams: {fileID: 0}\n"
	, ID, Owner->ID, CastShadows, MaterialText.c_str() );

	std::string RetStr = Text;

	delete[] Text;

	return RetStr;
}
std::string MeshFilter::GetSceneData()
{
	char Text[ 1024 ];
	sprintf_s( Text,
	"--- !u!33 &%lld\n"
	"MeshFilter:\n"
	"  m_ObjectHideFlags: 0\n"
	"  m_CorrespondingSourceObject: {fileID: 0}\n"
	"  m_PrefabInstance: {fileID: 0}\n"
	"  m_PrefabAsset: {fileID: 0}\n"
	"  m_GameObject: {fileID: %lld}\n"
	"  m_Mesh: {fileID: %lld, guid: %s, type: 3}\n"
	, this->ID, Owner->ID, SubMeshID, Mesh->GUID.c_str() );

	std::string RetStr = Text;

	return RetStr;
}
std::string LightComponent::GetSceneData()
{
	char Text[ 2048 ];
	int EnabledInt = (Enabled == true  ? 1: 0);
	int ShadowsType = ( Shadows == true ? 2 : 0 );
	sprintf_s( Text,
			   "--- !u!108 &%lld\n"
			   "Light:\n"
			   "  m_ObjectHideFlags: 0\n"
			   "  m_CorrespondingSourceObject: {fileID: 0}\n"
			   "  m_PrefabInstance: {fileID: 0}\n"
			   "  m_PrefabAsset: {fileID: 0}\n"
			   "  m_GameObject: {fileID: %lld}\n"
			   "  m_Enabled: %d\n"
			   "  serializedVersion: 8\n"
			   "  m_Type: %d\n"
			   "  m_Color: {r: %f, g: %f, b: %f, a: %f}\n"
			   "  m_Intensity: %f\n"
			   "  m_Range: %f\n"
			   "  m_SpotAngle: %f\n"
			   "  m_CookieSize: 10\n"
			   "  m_Shadows:\n"
			   "    m_Type: %d\n"
			   "    m_Resolution: -1\n"
			   "    m_CustomResolution: -1\n"
			   "    m_Strength: 1\n"
			   "    m_Bias: 0.05\n"
			   "    m_NormalBias: 0.4\n"
			   "    m_NearPlane: 0.2\n"
			   "  m_Cookie: {fileID: 0}\n"
			   "  m_DrawHalo: 0\n"
			   "  m_Flare: {fileID: 0}\n"
			   "  m_RenderMode: 0\n"
			   "  m_CullingMask:\n"
			   "    serializedVersion: 2\n"
			   "    m_Bits: 4294967295\n"
			   "  m_Lightmapping: 4\n"
			   "  m_LightShadowCasterMode: 0\n"
			   "  m_AreaSize: {x: 1, y: 1}\n"
			   "  m_BounceIntensity: 1\n"
			   "  m_ColorTemperature: 6570\n"
			   "  m_UseColorTemperature: 0\n"
			   "  m_ShadowRadius: 0\n"
			   "  m_ShadowAngle: 0\n",
			   this->ID, Owner->ID, EnabledInt, Type,
			   Color.R, Color.G, Color.B, Color.A,
			   Intensity, Range, SpotAngle, ShadowsType );

	std::string RetStr = Text;

	return RetStr;
}

void UnityTexture::GenerateGUID()
{
	std::size_t hash = std::hash<std::string>{}( File );
	GUID = GetGUIDFromSizeT( hash );
}
void UnityMesh::GenerateGUID()
{
	std::size_t hash = std::hash<std::string>{}( Name );
	GUID = GetGUIDFromSizeT( hash );
}
void UnityMaterial::GenerateShaderGUID()
{
	std::size_t hash = std::hash<std::string>{}( ShaderFileName );
	ShaderGUID = GetGUIDFromSizeT( hash );
}
void UnityMaterial::GenerateGUID()
{
	std::string Input = Name + ShaderGUID;
	std::size_t hash = std::hash<std::string>{}( Input );
	GUID = GetGUIDFromSizeT( hash );
}
GameObject::GameObject( )
{
	ID = Component::GetUniqueID();

	TransformComponent *Trans = new TransformComponent;
	Trans->Owner = this;
	Components.push_back( Trans );
}
Component* GameObject::AddComponent( ComponentType Type )
{
	if( Type == CT_MESHFILTER )
	{
		MeshFilter *MF = new MeshFilter;
		MF->Owner = this;
		Components.push_back( MF );
		return MF;
	}
	else if( Type == CT_MESHRENDERER )
	{
		MeshRenderer *MR = new MeshRenderer;
		MR->Owner = this;
		Components.push_back( MR );
		return MR;
	}
	else if( Type == CT_MESHCOLLIDER )
	{
		//MeshRenderer *MR = new MeshRenderer;
		//Components.push_back( MR );
		//return MR;
	}
	else if( Type == CT_LIGHT )
	{
		LightComponent* NewLight = new LightComponent;
		NewLight->Owner = this;
		Components.push_back( NewLight );
		return NewLight;
	}

	return nullptr;
}
void GameObject::AddChild( GameObject* GO )
{
	Children.push_back( GO );
	GO->Parent = this;
}
Component *GameObject::GetComponent( ComponentType Type )
{
	for ( int i = 0; i < Components.size(); i++ )
	{
		if ( Components[ i ]->Type == Type )
			return Components[ i ];
	}

	return nullptr;
}

std::string GameObject::ToSceneData()
{
	std::string RetStr;

	std::string MainStr = GetGameObjectString();
	RetStr += MainStr;

	for( int i = 0; i < Components.size(); i++ )
	{
		auto Comp = Components[ i ];
		//if( Comp->Type == ComponentType::CT_TRANSFORM )
			RetStr += Comp->GetSceneData();
	}

	for( int i = 0; i < Children.size(); i++ )
	{
		auto Child = Children[ i ];
		std::string ChildStr = Child->ToSceneData();
		RetStr += ChildStr;
	}

	return RetStr;
}
std::string GameObject::GetGameObjectString()
{
	int serializedVersion = 6;// 4;
	
	char Text[ 1024 ];
	sprintf_s( Text,
		"--- !u!1 &%lld\n"
		"GameObject:\n"
		"  m_ObjectHideFlags: 0\n"
		"  m_CorrespondingSourceObject: {fileID: 0}\n"
		"  m_PrefabInstance: {fileID: 0}\n"
		"  m_PrefabAsset: {fileID: 0}\n"
		"  serializedVersion: %d\n"
		"  m_Component:\n",
			 this->ID, serializedVersion );

	std::string Str = Text;

	for ( int i = 0; i < Components.size(); i++ )
	{
		auto Comp = Components[ i ];
		char Line[ 128 ];
		if ( serializedVersion >= 5 )
		{
			sprintf_s( Line, "  - component: {fileID: %lld}\n", Comp->ID );
		}		
		
		Str += Line;
	}
	
	sprintf_s( Text,
	"  m_Layer: 0\n"
	"  m_Name: %s\n"
	"  m_TagString: Untagged\n"
	"  m_Icon: {fileID: 0}\n"
	"  m_NavMeshLayer: 0\n"
	"  m_StaticEditorFlags: 0\n"
	"  m_IsActive: 1\n",
	this->Name.c_str());

	Str += Text;

	return Str;
}
void Scene::Add( GameObject* GO )
{
	GameObjects.push_back( GO );
	GO->OwnerScene = this;
}
void Scene::WriteScene( const wchar_t *File, const wchar_t *ProxyScene )
{
	FILE *f = nullptr;
	_wfopen_s(&f, File, L"w" );
	if( !f )
	{
		return;
	}

	std::string SceneProxy;
	std::string Str = SceneProxy;
	
	for( int i = 0; i < GameObjects.size(); i++ )
	{
		GameObject *GO = GameObjects[ i ];
		if( !GO->Parent )
		{
			std::string TextData = GO->ToSceneData();
			Str += TextData;
		}
	}

	unsigned char *ProxySceneData = nullptr;
	int Size = LoadFile( ToANSIString( ProxyScene ).c_str(), &ProxySceneData );
	if( !ProxySceneData || Size <= 0 )
	{
		UE_LOG( LogTemp, Error, TEXT( "LoadFile( %s ) failed" ), ProxyScene );
		return;
	}

	std::string FinalSceneData = (char*)ProxySceneData;
	
	FinalSceneData += Str;

	fprintf( f, "%s", FinalSceneData.c_str() );
	fclose( f );
}
void Scene::WriteMaterials()
{

}

int GUIDCounter = 0;
std::string GetGUIDFromSizeT( size_t Input )
{
	//char Base[] = "c5890e70e509bae42997d0b1b0696b6a";
	  char Base[] = "abc00000000000000000000000000000";
	//7338d2bd21532424daf2ad06ccf3a92c
	char Num[ 32 ];
	sprintf_s( Num, "%Iu", (unsigned __int64)Input );// GUIDCounter );
	int NumLen = strlen( Num );
	std::string GUID = Base;
	for( int i = 32 - NumLen,u = 0; i < 32; i++, u++ )
	{
		GUID[ i ] = Num[ u ];
	}
	
	//GUIDCounter++;

	return GUID;
}

enum FileIDType
{
	FIT_MATERIALS,
	FIT_MESHES,
};
class FileID
{
public:
	FileID( int pBaseID, const char* str, int pIndex = 0)
	{
		BaseID = pBaseID;
		//Type = pType;
		Identifier = str;
		Index = pIndex;
	}
	int BaseID;
	//FileIDType Type;
	int Index;
	std::string Identifier;

	static int GetBase( FileIDType Type )
	{
		switch( Type )
		{
			case FIT_MATERIALS: return 2100000;
			case FIT_MESHES	  : return 4300000;
			default:return Type;
		}

		return Type;
	}
	std::string GetMetaLine()
	{
		//int Base = GetBase( Type );
		int ID = BaseID + Index * 2;
		char Line[ 1024 ];
		sprintf_s( Line, "    %d: %s\n", ID, Identifier.c_str() );

		return Line;
	}
};

std::string GenerateMeshMetaFileIDs( UnityMesh* M )
{
	const char* MeshName = M->Name.c_str();
	int SubMeshes = M->Sections.size();

	std::string Ret;	
	if( SubMeshes <= 1 )
	{
		char Text[ 4096 ] = "";
		sprintf_s( Text,
		"    100000: %s\n"
		"    100002: //RootNode\n"
		"    400000: %s\n"
		"    400002: //RootNode\n"
		"    2300000: %s\n"
		"    3300000: %s\n"
		"    4300000: %s\n",
				   MeshName, MeshName, MeshName, MeshName, MeshName  );
		return  Text;
	}
	int TextLength = 1024 + SubMeshes * 128;
	char *Text = new char[ TextLength ];

	sprintf_s( Text, TextLength,
	"    100000: %s\n"
	"    100002: //RootNode\n",
	MeshName );
	Ret += Text;

	std::vector<FileID> FileIDArray;

	//100000: //RootNode
	//400000 : //RootNode
	//2100000 : MI_BS_Screens
	//2100002 : MI_BS_TV_Screen_01
	//2300000 : //RootNode
	//3300000 : //RootNode
	//4300000 : SM_BS_Screen_01_2

	FileIDArray.push_back( FileID( 100000, "//RootNode", 0 ) );
	FileIDArray.push_back( FileID( 400000, "//RootNode", 0 ) );

	for( int i = 0; i < M->Materials.size(); i++ )
	{
		if ( M->Materials[i] )
			FileIDArray.push_back( FileID( 2100000, M->Materials[i]->Name.c_str(), i ) );
	}

	FileIDArray.push_back( FileID( 2300000, "//RootNode", 0 ) );
	FileIDArray.push_back( FileID( 3300000, "//RootNode", 0 ) );

	FileIDArray.push_back( FileID( 4300000, M->Name.c_str(), 0 ) );
	
	for( int i = 0; i < FileIDArray.size(); i++ )
	{
		std::string Line = FileIDArray[ i ].GetMetaLine();
		Ret += Line;
	}

	//uint64 BaseNumbers[] = { 100004, 400004, 2300002, 3300002, 4300002 };
	//int IDTypes = 5;
	//for( int u = 0; u < IDTypes; u++ )
	//{
	//	if( u == 1 )
	//	{
	//		sprintf_s( Text, TextLength,
	//				   "    400000: %s\n"
	//				   "    400002: //RootNode\n",
	//				   MeshName );
	//
	//		Ret += Text;
	//	}
	//	if( u == 2 )
	//	{
	//		sprintf_s( Text, TextLength,
	//				   "    2300000: %s\n",
	//				   MeshName );
	//
	//		Ret += Text;
	//	}
	//	if( u == 3 )
	//	{
	//		sprintf_s( Text, TextLength,
	//				   "    3300000: %s\n",
	//				   MeshName );
	//
	//		Ret += Text;
	//	}
	//	/*if( u == 4 )
	//	{
	//		sprintf_s( Text,
	//				   "    3300000: %s\n",
	//				   MeshName );
	//
	//		Ret += Text;
	//	}*/
	//
	//	for( int i = 0; i < SubMeshes; i++ )
	//	{
	//		char Line[ 128 ];
	//
	//		uint64_t Number = BaseNumbers[ u ] + i * 2;
	//		sprintf( Line, "    %lld: %s.%d\n", Number, MeshName, i );
	//		Ret += Line;
	//	}
	//}
	//
	//delete[] Text;

	return Ret;
}

std::string GenerateOBJMeta( UnityMesh* Mesh )
{
	const char* MeshName = Mesh->Name.c_str();
	int SubMeshes = Mesh->Sections.size();
	//std::string * GUID = &Mesh->GUID;
	Mesh->GenerateGUID();

	std::string MetaFileIDs = GenerateMeshMetaFileIDs( Mesh );
	
	//*GUID = GenerateGUID();
	int TextLength = 4096 + MetaFileIDs.length();
	char *Text = new char[ TextLength ];
	sprintf_s( Text, TextLength,
"fileFormatVersion: 2\n"
"guid: %s\n"
"timeCreated: 1501192767\n"
"licenseType: Free\n"
"ModelImporter:\n"
"  serializedVersion: 21\n"
"  fileIDToRecycleName:\n"
"%s"//MetaFileIDs
"  materials:\n"
"    importMaterials: 1\n"
"    materialName: 0\n"
"    materialSearch: 1\n"
"  animations:\n"
"    legacyGenerateAnimations: 4\n"
"    bakeSimulation: 0\n"
"    resampleCurves: 1\n"
"    optimizeGameObjects: 0\n"
"    motionNodeName: \n"
"    rigImportErrors: \n"
"    rigImportWarnings: \n"
"    animationImportErrors: \n"
"    animationImportWarnings: \n"
"    animationRetargetingWarnings: \n"
"    animationDoRetargetingWarnings: 0\n"
"    animationCompression: 1\n"
"    animationRotationError: 0.5\n"
"    animationPositionError: 0.5\n"
"    animationScaleError: 0.5\n"
"    animationWrapMode: 0\n"
"    extraExposedTransformPaths: []\n"
"    clipAnimations: []\n"
"    isReadable: 1\n"
"  meshes:\n"
"    lODScreenPercentages: []\n"
"    globalScale: 1\n"
"    meshCompression: 0\n"
"    addColliders: 0\n"
"    importBlendShapes: 1\n"
"    swapUVChannels: 0\n"
"    generateSecondaryUV: 0\n"
"    useFileUnits: 1\n"
"    optimizeMeshForGPU: 0\n"
"    keepQuads: 0\n"
"    weldVertices: 0\n"
"    preserveHierarchy: 0\n"
"    indexFormat : 2\n"
"    secondaryUVAngleDistortion: 8\n"
"    secondaryUVAreaDistortion: 15.000001\n"
"    secondaryUVHardAngle: 88\n"
"    secondaryUVPackMargin: 4\n"
"    useFileScale: 1\n"
"  tangentSpace:\n"
"    normalSmoothAngle: 60\n"
"    normalImportMode: 0\n"
"    tangentImportMode: 0\n"
"  importAnimation: 1\n"
"  copyAvatar: 0\n"
"  humanDescription:\n"
"    serializedVersion: 2\n"
"    human: []\n"
"    skeleton: []\n"
"    armTwist: 0.5\n"
"    foreArmTwist: 0.5\n"
"    upperLegTwist: 0.5\n"
"    legTwist: 0.5\n"
"    armStretch: 0.05\n"
"    legStretch: 0.05\n"
"    feetSpacing: 0\n"
"    rootMotionBoneName: \n"
"    rootMotionBoneRotation: {x: 0, y: 0, z: 0, w: 1}\n"
"    hasTranslationDoF: 0\n"
"    hasExtraRoot: 0\n"
"    skeletonHasParents: 1\n"
"  lastHumanDescriptionAvatarSource: {instanceID: 0}\n"
"  animationType: 0\n"
"  humanoidOversampling: 1\n"
"  additionalBone: 0\n"
"  userData: \n"
"  assetBundleName: \n"
"  assetBundleVariant:\n"

	, Mesh->GUID.c_str(), MetaFileIDs.c_str() );

	std::string Ret;
	Ret = Text;
	delete[] Text;
	return Ret;
}

std::string  UnityMaterial::GenerateMaterialFile( )
{
	int serializedVersion = 3;//2
	std::string Ret;
	char Text[ 2048 ] = "";
	sprintf_s( Text, 
"%%YAML 1.1\n"
"%%TAG !u! tag:unity3d.com,2011:\n"
"--- !u!21 &2100000\n"
"Material:\n"
"  serializedVersion: 6\n"
"  m_ObjectHideFlags: 0\n"
"  m_CorrespondingSourceObject: {fileID: 0}\n"
"  m_PrefabInstance: {fileID: 0}\n"
"  m_PrefabAsset: {fileID: 0}\n"
"  m_Name: SM_ChairMat\n"
"  m_Shader: {fileID: 4800000, guid: %s, type: 3}\n"
"  m_ShaderKeywords: \n"
"  m_LightmapFlags: 4\n"
"  m_EnableInstancingVariants: 0\n"
"  m_DoubleSidedGI: 0\n"
"  m_CustomRenderQueue: -1\n"
"  stringTagMap: {}\n"
"  disabledShaderPasses: []\n"
"  m_SavedProperties:\n"
"    serializedVersion: %d\n"
"    m_TexEnvs:\n", ShaderGUID.c_str(), serializedVersion );
	Ret += Text;

	for(int i=0; i<Textures.size(); i++ )
	{
		auto Tex = Textures[ i ];

		if ( serializedVersion == 3 )//Unity 2017.1
		{
			sprintf_s( Text,
			"    - Material_Texture2D_%d:\n"
			"        m_Texture: {fileID: 2800000, guid: %s, type: 3}\n"
			"        m_Scale: {x: 1, y: 1}\n"
			"        m_Offset: {x: 0, y: 0}\n",
					i,
					Tex->GUID.c_str() );
		}
		else if ( serializedVersion == 2 )//Unity 5.4.3
		{
			sprintf_s( Text,
			"    - first:\n"
			"        name: Material_Texture2D_%d\n"
			"      second :\n"
			"        m_Texture: {fileID: 2800000, guid : %s, type : 3}\n"
			"        m_Scale: {x: 1, y : 1}\n"
			"        m_Offset: {x: 0, y : 0}\n",
				i,
				Tex->GUID.c_str() );
		}

		Ret += Text;
	}

	sprintf_s( Text,
"    m_Floats:\n"
"    - _DstBlend: 0\n"
"    - _SrcBlend: 1\n"
"    - _UVSec: 0\n"
"    - _ZWrite: 1\n"
"    m_Colors:\n"
"    - _Color: {r: 0.8, g: 0.8, b: 0.8, a: 1}\n"
"    - _EmissionColor: {r: 0, g: 0, b: 0, a: 1}\n");

	Ret += Text;

	return Ret;
}
std::string  GenerateMaterialMeta( std::string GUID )
{
	//GUID = GenerateGUID();

	char Text[ 2048 ] = "";
	sprintf_s( Text,
"fileFormatVersion: 2\n"
"guid: %s\n"
"timeCreated: 1501487310\n"
"licenseType: Free\n"
"NativeFormatImporter:\n"
"  mainObjectFileID: 2100000\n"
"  userData: \n"
"  assetBundleName: \n"
"  assetBundleVariant: \n",
	GUID.c_str() );

	return Text;
}

std::string  GenerateShaderMeta( std::string GUID )
{
	//GUID = GenerateGUID();
	char Text[ 2048 ] = "";
	sprintf_s( Text,
"fileFormatVersion: 2\n"
"guid: %s\n"
"timeCreated: 1501487310\n"
"licenseType: Free\n"
"ShaderImporter:\n"
"  defaultTextures: []\n"
"  userData: \n"
"  assetBundleName: \n"
"  assetBundleVariant: \n",
GUID.c_str() );

	return Text;
}

std::string  GenerateTextureMeta( std::string GUID, bool IsNormalMap, int sRGB )
{
	char Text[ 2048 ] = "";

	//int sRGB = 1;
	int texturetype = 0;
	
	if( IsNormalMap )
	{
		//sRGB = 0;
		texturetype = 1;
	}

	int serializedVersion = 4;
	if ( serializedVersion == 4 )
	{
	sprintf_s( Text,
	"fileFormatVersion: 2\n"
	"guid: %s\n"
	"timeCreated: 1501492331\n"
	"licenseType: Free\n"
	"TextureImporter:\n"
	"  fileIDToRecycleName: {}\n"
	"  serializedVersion: 4\n"
	"  mipmaps:\n"
	"    mipMapMode: 0\n"
	"    enableMipMap: 1\n"
	"    sRGBTexture: %d\n"
	"    linearTexture: 0\n"
	"    fadeOut: 0\n"
	"    borderMipMap: 0\n"
	"    mipMapsPreserveCoverage: 0\n"
	"    alphaTestReferenceValue: 0.5\n"
	"    mipMapFadeDistanceStart: 1\n"
	"    mipMapFadeDistanceEnd: 3\n"
	"  bumpmap:\n"
	"    convertToNormalMap: 0\n"
	"    externalNormalMap: 0\n"
	"    heightScale: 0.25\n"
	"    normalMapFilter: 0\n"
	"  isReadable: 0\n"
	"  grayScaleToAlpha: 0\n"
	"  generateCubemap: 6\n"
	"  cubemapConvolution: 0\n"
	"  seamlessCubemap: 0\n"
	"  textureFormat: 1\n"
	"  maxTextureSize: 2048\n"
	"  textureSettings:\n"
	"    serializedVersion: 2\n"
	"    filterMode: -1\n"
	"    aniso: -1\n"
	"    mipBias: -1\n"
	"    wrapU: -1\n"
	"    wrapV: -1\n"
	"    wrapW: -1\n"
	"  nPOTScale: 1\n"
	"  lightmap: 0\n"
	"  compressionQuality: 50\n"
	"  spriteMode: 0\n"
	"  spriteExtrude: 1\n"
	"  spriteMeshType: 1\n"
	"  alignment: 0\n"
	"  spritePivot: {x: 0.5, y: 0.5}\n"
	"  spriteBorder: {x: 0, y: 0, z: 0, w: 0}\n"
	"  spritePixelsToUnits: 100\n"
	"  alphaUsage: 1\n"
	"  alphaIsTransparency: 0\n"
	"  spriteTessellationDetail: -1\n"
	"  textureType: %d\n"
	"  textureShape: 1\n"
	"  maxTextureSizeSet: 0\n"
	"  compressionQualitySet: 0\n"
	"  textureFormatSet: 0\n"
	"  platformSettings:\n"
	"  - buildTarget: DefaultTexturePlatform\n"
	"    maxTextureSize: 8192\n"
	"    textureFormat: -1\n"
	"    textureCompression: 1\n"
	"    compressionQuality: 50\n"
	"    crunchedCompression: 0\n"
	"    allowsAlphaSplitting: 0\n"
	"    overridden: 0\n"
	"  spriteSheet:\n"
	"    serializedVersion: 2\n"
	"    sprites: []\n"
	"    outline: []\n"
	"    physicsShape: []\n"
	"  spritePackingTag: \n"
	"  userData: \n"
	"  assetBundleName: \n"
	"  assetBundleVariant: \n",
				   GUID.c_str(), sRGB, texturetype );
	}
	else if ( serializedVersion == 2 )
	{
		int textureformat = -3;
		
		if ( IsNormalMap )
		{
			textureformat = -1;
		}

		sprintf_s( Text,
			"fileFormatVersion: 2\n"
			"guid: %s\n"
			"TextureImporter:\n"
			"  fileIDToRecycleName: {}\n"
			"  serializedVersion: 2\n"
			"  mipmaps:\n"
			"    mipMapMode: 0\n"
			"    enableMipMap: 1\n"
			"    linearTexture: 1\n"
			"    correctGamma: 0\n"
			"    fadeOut: 0\n"
			"    borderMipMap: 0\n"
			"    mipMapFadeDistanceStart: 1\n"
			"    mipMapFadeDistanceEnd: 3\n"
			"  bumpmap:\n"
			"    convertToNormalMap: 0\n"
			"    externalNormalMap: 1\n"
			"    heightScale: 0.25\n"
			"    normalMapFilter: 0\n"
			"  isReadable: 1\n"
			"  grayScaleToAlpha: 0\n"
			"  generateCubemap: 0\n"
			"  cubemapConvolution: 0\n"
			"  cubemapConvolutionSteps: 7\n"
			"  cubemapConvolutionExponent: 1.5\n"
			"  seamlessCubemap: 0\n"
			"  textureFormat: %d\n"
			"  maxTextureSize: 8192\n"
			"  textureSettings:\n"
			"    filterMode: -1\n"
			"    aniso: -1\n"
			"    mipBias: -1\n"
			"    wrapMode: -1\n"
			"  nPOTScale: 1\n"
			"  lightmap: 0\n"
			"  rGBM: 0\n"
			"  compressionQuality: 50\n"
			"  allowsAlphaSplitting: 0\n"
			"  spriteMode: 0\n"
			"  spriteExtrude: 1\n"
			"  spriteMeshType: 1\n"
			"  alignment: 0\n"
			"  spritePivot: {x: 0.5, y: 0.5}\n"
			"  spriteBorder: {x: 0, y: 0, z: 0, w: 0}\n"
			"  spritePixelsToUnits: 100\n"
			"  alphaIsTransparency: 0\n"
			"  spriteTessellationDetail: -1\n"
			"  textureType: %d\n"
			"  buildTargetSettings: []\n"
			"  spriteSheet:\n"
			"    serializedVersion: 2\n"
			"    sprites: []\n"
			"    outline: []\n"
			"  spritePackingTag: \n"
			"  userData: \n"
			"  assetBundleName: \n"
			"  assetBundleVariant: \n",
			GUID.c_str(), textureformat, texturetype );
	}
	return Text;
}
std::string  GenerateExportedFBXMeta( )
{	
	char Text[ 2048 ] = "";
	std::string GUID = GetGUIDFromSizeT(1);
	sprintf_s( Text,
				"fileFormatVersion: 2\n"
				"guid: %s\n"
				"timeCreated: 1505388383\n"
				"licenseType: Free\n"
				"ModelImporter:\n"
				"  serializedVersion: 21\n"
				"  materials:\n"
				"    importMaterials: 1\n"
				"    materialName: 1\n"
				"    materialSearch: 1\n", GUID.c_str() );

	return Text;
}
#endif