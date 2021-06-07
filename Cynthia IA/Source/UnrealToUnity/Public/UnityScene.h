#pragma once

#include <string>
#include <vector>
enum ComponentType
{
	CT_UNKNOWN = 0,
	CT_TRANSFORM,
	CT_MESHRENDERER,
	CT_MESHFILTER,
	CT_MESHCOLLIDER,
	CT_LIGHT
};
class Component
{
public :
	Component();
	virtual ~Component() {};
	uint64 ID;
	ComponentType Type;
	std::string Flag;
	bool Enabled;
	std::string Name;
	class GameObject *Owner = nullptr;

	static uint64 Counter;
	virtual std::string GetSceneData();
	static uint64 GetUniqueID();
};

class TransformComponent : public Component
{
public:
	TransformComponent();
	void Reset();

	float LocalPosition[ 3 ];
	float LocalRotation[ 4 ];//Quat ?
	float LocalScale[ 3 ];

	std::string GetChildrenText();
	std::string GetSceneData();
};
class MeshRenderer : public Component
{
public:
	MeshRenderer()
	{
		Type = CT_MESHRENDERER;
	}
	
	std::string GetSceneData();
	std::vector<class UnityMaterial *> Materials;
	bool CastShadows = true;
};

class UnityTexture
{
public:
	void GenerateGUID();

	std::string File;
	std::string GUID;
};

class UnityMaterial
{
public:
	void GenerateGUID();
	void GenerateShaderGUID();

	std::string Name;
	std::string GUID;
	std::string ShaderGUID;
	std::string ShaderFileName;
	std::string ShaderContents;	

	std::vector< UnityTexture* > Textures;	
	std::string GenerateMaterialFile();
};

struct MeshSection
{
	//UINT *Indices = nullptr;
	UINT IndexOffset = 0;
	UINT NumIndices = 0;
	UINT MaterialIndex = 0;
};

class UnityMesh
{
public:
	void GenerateGUID();

	std::string Name;
	std::string GUID;

	struct FVector*		Vertices = nullptr;
	struct FVector*		Normals = nullptr;
	struct FVector*		Tangents = nullptr;
	struct FVector2D*	Texcoords = nullptr;
	struct FVector2D*	Texcoords1 = nullptr;
	struct FVector2D*	Texcoords2 = nullptr;
	struct FVector2D*	Texcoords3 = nullptr;
	struct FColor*		Colors = nullptr;
	UINT *AllIndices = nullptr;
	std::vector<MeshSection*> Sections;

	int NumVertices;
	int NumIndices;

	std::vector<UnityMaterial*> Materials;
};

class MeshFilter : public Component
{
public:
	MeshFilter()
	{
		Type = CT_MESHFILTER;
	}
	UnityMesh *Mesh = nullptr;
	
	uint64_t SubMeshID = 4300000;
	std::string GetSceneData();
};

enum UnityLightType
{
	LT_SPOT,
	LT_DIRECTIONAL,
	LT_POINT,
	LT_AREA
};

class LightComponent : public Component
{
public:
	LightComponent()
	{
		Component::Type = CT_LIGHT;
	}
	std::string GetSceneData();
	
	UnityLightType Type;
	float Range;
	FLinearColor Color;
	float Intensity;
	float SpotAngle = 30.0f;
	bool Shadows = true;
};
class Scene;
class GameObject
{
public:
	GameObject( );

	Component *GetComponent( ComponentType Type );
	Component* AddComponent( ComponentType Type );
	void AddChild( GameObject* GO );

	std::string GetGameObjectString();
	std::string ToSceneData();

	std::vector< Component* > Components;
	std::vector< GameObject* > Children;
	GameObject *Parent = nullptr;
	std::string Name;
	uint64 ID;
	Scene* OwnerScene = nullptr;
};

class Scene
{
public:
	std::vector< GameObject* > GameObjects;
	std::vector< UnityMaterial* > Materials;

	void Add( GameObject* GO );

	void WriteScene( const wchar_t *File, const wchar_t *ProxyScene );
	void WriteMaterials();
};

std::string GenerateOBJMeta( UnityMesh* M );
int64_t LoadFile( const char *File, unsigned char **data );
std::string ToANSIString( std::wstring w );
std::string GenerateMaterialMeta( std::string GUID );
std::string GenerateShaderMeta( std::string GUID );
std::string GenerateTextureMeta( std::string GUID, bool IsNormalMap, int sRGB );
std::string GetGUIDFromSizeT( size_t Input );
std::string GenerateExportedFBXMeta();
extern int GUIDCounter;