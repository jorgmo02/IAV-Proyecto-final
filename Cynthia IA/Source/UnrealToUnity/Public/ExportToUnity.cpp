

#include "ExportToUnity.h"

#include "UnityScene.h"


#include "Runtime/Core/Public/GenericPlatform/GenericPlatformFile.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "EngineUtils.h"
#include "StaticMeshResources.h"
#include "ImageUtils.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"

#include "Runtime/Engine/Classes/Components/LightComponent.h"
#include "Runtime/Engine/Classes/Components/PointLightComponent.h"
#include "Runtime/Engine/Classes/Components/SpotLightComponent.h"
#include "Runtime/Engine/Classes/Components/DirectionalLightComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "Runtime/Engine/Private/Materials/MaterialUniformExpressions.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstance.h"
//#include "Runtime/Engine/Private/Materials/HLSLMaterialTranslator.h"//for num texcoords
#include "Runtime/Engine/Private/DebugViewModeMaterialProxy.h"//for the class that triggers material instance recompilation


#include "Runtime/Engine/Classes/Materials/MaterialExpressionMultiply.h"
#include "Runtime/Engine/Classes/Materials/MaterialExpressionStaticSwitchParameter.h"
#include "Runtime/Engine/Classes/Materials/MaterialExpressionConstant.h"
#include "Runtime/Engine/Classes/Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Runtime/Engine/Classes/Materials/MaterialExpressionMakeMaterialAttributes.h"
#include "Runtime/Engine/Classes/Materials/MaterialExpressionCollectionParameter.h"
#include "Runtime/Engine/Classes/Materials/MaterialParameterCollection.h"


#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/Paths.h"

#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"


ERHIFeatureLevel::Type GlobalFeatureLevel = ERHIFeatureLevel::SM5;
EMaterialQualityLevel::Type GlobalQualityLevel = EMaterialQualityLevel::High;

#if WITH_EDITOR


#include "MaterialShared.generated.h"
#include "Editor/MaterialEditor/Public/MaterialEditorUtilities.h"

#include "UnrealEdGlobals.h"//for GUnrealEd
#include "Editor/UnrealEdEngine.h"

void ConvertTransform( FTransform Trans, TransformComponent* Transform, bool IsRelative = false );

void FromUnicode( const wchar_t *in, char *Out )
{
	//+1 makes it equal the 0 at the end
	int len = ( int )wcslen( in ) * 2;//+1;
	char *p = ( char* )in;
	for ( int y = 0; y<len; y += 2 )
	{
		Out[ y / 2 ] = p[ y ];
	}
}
const char *FromWideString( const wchar_t *Unicode )
{
	if ( !Unicode )
		return NULL;

	int length = ( int )wcslen( Unicode );
	char *Text = new char[ length + 1 ];
	Text[ length ] = 0;//sanity check

	FromUnicode( Unicode, Text );
	return Text;
}
std::wstring ToWideString( std::string str )
{
	std::wstring w;
	for( int i = 0; i < str.length(); i++ )
	{
		w += str[ i ];
	}
	return w;
}
std::string ToANSIString( std::wstring w )
{
	std::string s;
	for( int i = 0; i < w.length(); i++ )
	{
		s += w[ i ];
	}
	return s;
}

int TotalMeshes = 0;
int TotalTriangles = 0;

bool ExportMeshes = true;// false;
bool ExportTextures = true;
bool ForceShaderRecompilation = false;
bool ClearMetas = true;

wchar_t UnityProjectFolder[1024];
//const wchar_t *GetUnityAssetsFolder() = L"C:\\UnrealToUnity\\Assets\\";
std::wstring GetUnityAssetsFolder()
{
	std::wstring Folder = UnityProjectFolder;
	Folder += L"Assets\\";

	return Folder;
}

Scene *UnityScene = nullptr;

class TextureBinding
{
public:
	UnityTexture *UnityTex = nullptr;
	UTexture *UnrealTexture = nullptr;
	bool IsNormalMap = false;
};

std::vector< TextureBinding* > AllTextures;

std::wstring GenerateTexturePath( UTexture *Tex )
{
	std::wstring TextureFileName = GetUnityAssetsFolder();
	TextureFileName += L"Textures\\";
	TextureFileName += *Tex->GetName();
	TextureFileName += L".png";

	return TextureFileName;
}

class UETextureExporter
{
public:	
	static void DoExport( UTexture *Tex )
	{
		std::wstring TextureFileName = GenerateTexturePath( Tex );
		
		if( !ExportTextures )
			return;
		//Don't save duplicates !
		if ( !FPlatformFileManager::Get().GetPlatformFile().FileExists( TextureFileName.c_str() ) )
		{
			TArray64<uint8> OutMipData;
			ETextureSourceFormat Format = Tex->Source.GetFormat();

			if ( Format != TSF_BGRA8 && Format != TSF_RGBA8 && Format != TSF_RGBA16 && Format != TSF_G8 )
			{
				UE_LOG( LogClass, Log, TEXT("UETextureExporter::DoExport Format %d is not standard!"), Format );
				return;
			}

			int32 Width = Tex->Source.GetSizeX();
			int32 Height = Tex->Source.GetSizeY();

			Tex->Source.GetMipData( OutMipData, 0 );

			TArray<uint8> PNG_Compressed_ImageData;

			TArray<FColor> InputImageData;

			if( Format == TSF_G8 )
			{
				for( int i = 0; i < OutMipData.Num(); i++ )
				{					
					FColor Color( OutMipData[ i ], OutMipData[ i ], OutMipData[ i ], OutMipData[ i ] );
					InputImageData.Add( Color );
				}
			}
			else if( Format == TSF_RGBA16 )
			{
				for( int i = 0; i < OutMipData.Num(); i += 8 )
				{
					FColor Color;
					uint16 R16 = OutMipData[ i + 0 ] * 256 + OutMipData[ i + 1 ];
					uint16 G16 = OutMipData[ i + 2 ] * 256 + OutMipData[ i + 3 ];
					uint16 B16 = OutMipData[ i + 4 ] * 256 + OutMipData[ i + 5 ];
					uint16 A16 = OutMipData[ i + 6 ] * 256 + OutMipData[ i + 7 ];
					Color.R = R16 / 256;
					Color.G = G16 / 256;
					Color.B = B16 / 256;
					Color.A = A16 / 256;
					InputImageData.Add( Color );
				}
			}
			else
			{
				for( int i = 0; i < OutMipData.Num(); i += 4 )
				{
					FColor Color(	  OutMipData[ i + 2 ], OutMipData[ i + 1 ], OutMipData[ i + 0 ], OutMipData[ i + 3 ] );
					//FColor ColorBGRA( OutMipData[ i + 0 ], OutMipData[ i + 2 ], OutMipData[ i + 1 ], OutMipData[ i + 3 ] );
					//if ( Format == TSF_BGRA8 )
						//InputImageData.Add( ColorBGRA );
					//else
						InputImageData.Add( Color );
				}
			}

			//~~~~~~~~~~~~~~~~
			// Compress to PNG
			//~~~~~~~~~~~~~~~~
			FImageUtils::CompressImageArray(
				Width,
				Height,
				InputImageData,
				PNG_Compressed_ImageData
				);

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			//                    Save binary array PNG image file to disk! 


			FFileHelper::SaveArrayToFile(
				PNG_Compressed_ImageData,
				TextureFileName.c_str()
				);
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		//~~~ Empty PNG Buffer ~~~
		//PNG_Compressed_ImageData.Empty();
		//~~~~~~~~~~~~~~~~~~~~~~~~

		//~~~ Empty Color Buffer ~~~
		//PNGScreenShot_ColorBuffer.Empty();
		//~~~~~~~~~~~~~~~~~~~~~~~~
	}
	static TextureBinding *Find( UTexture *Tex )
	{
		for( int i = 0; i < AllTextures.size(); i++ )
		{
			auto Binding = AllTextures[ i ];
			if( Binding->UnrealTexture == Tex )
				return Binding;
		}

		return nullptr;
	}
	static TextureBinding *Export( UTexture *T )
	{
		auto Existent = Find( T );
		if( Existent )
			return Existent;

		DoExport( T );

		TextureBinding *NewTextureBinding = new TextureBinding;
		NewTextureBinding->UnrealTexture = T;
		AllTextures.push_back( NewTextureBinding );

		return NewTextureBinding;
	}
};

std::vector< MaterialBinding *> AllMaterials;
MaterialBinding *GetMaterialIDIfAlreadyExported( UMaterialInterface *M )
{
	for ( int i = 0; i < AllMaterials.size(); i++ )
	{
		if ( AllMaterials[ i ]->MaterialInterface == M )
			return AllMaterials[ i ];
	}

	return nullptr;
}
MaterialBinding *ProcessMaterialReference( UMaterialInterface *M )
{
	MaterialBinding *Mat = GetMaterialIDIfAlreadyExported( M );

	if ( !Mat )
	{
		Mat = new MaterialBinding();
		Mat->MaterialInterface = M;
		Mat->ID = AllMaterials.size();
		AllMaterials.push_back( Mat );
	}
	
	return Mat;
}
FString GetProjectName()
{
	FString ProjectFilePath = FPaths::GetProjectFilePath();
	int PointStart = ProjectFilePath.Find( ".", ESearchCase::IgnoreCase, ESearchDir::FromEnd );
	int NameStart = ProjectFilePath.Find( "/", ESearchCase::IgnoreCase, ESearchDir::FromEnd );
	FString ProjectName = ProjectFilePath.Mid( NameStart + 1, PointStart - NameStart - 1 );
	return ProjectName;
}
FString GetResourceDir()
{
	FString ProjectDir = FPaths::ProjectDir();

	ProjectDir.Append( TEXT( "/Plugins/UnrealToUnity/Resources/" ) );

	return ProjectDir;
}
void CopyToOutput( const wchar_t *SourceFile, const wchar_t *OutFilePath )
{
	FString ProjectDir = FPaths::ProjectContentDir();
	
	FString ResourceDir = GetResourceDir();
	std::wstring SourceFilePath = *ResourceDir;
	SourceFilePath += SourceFile;
	std::wstring To = UnityProjectFolder;
	To += OutFilePath;

	bool Result = FPlatformFileManager::Get().GetPlatformFile().CopyFile( To.c_str(), SourceFilePath.c_str() );
	if( !Result )
	{
		UE_LOG( LogTemp, Error, TEXT( "CopyToOutput Error on SourceFile=%s OutFilePath=%s" ), SourceFilePath.c_str(), To.c_str() );
	}
}
void ProcessMaterial_RenderThread( MaterialBinding *Mat, const char *ProxyShaderString );
void ProcessAllMaterials( )
{
	FString ResourceDir = GetResourceDir();	
	std::wstring ProxyShader = *ResourceDir;
	ProxyShader += L"/Proxy.shader";

	CopyToOutput( L"UnrealCommon.cginc", L"Assets\\Shaders\\UnrealCommon.cginc" );
	
	const char *ProxyShaderString = nullptr;
	int Size = LoadFile( ToANSIString( ProxyShader ).c_str(), (BYTE**)&ProxyShaderString );
	if ( Size < 0 )
	{
		//ERROR!
		return;
	}
	for ( int i = 0; i < AllMaterials.size(); i++ )
	{
		ProcessMaterial_RenderThread( AllMaterials[ i ], ProxyShaderString );
	}
}
std::wstring GetOutputFile( FString SubFolder, FString Name, const wchar_t *Ext )
{
	std::wstring OutFolder = GetUnityAssetsFolder();
	OutFolder += *SubFolder;
	OutFolder += *Name;
	OutFolder += Ext;

	return OutFolder;
}
class FUniformExpressionSet_Override : public FUniformExpressionSet
{
public:
	
	TMemoryImageArray<FMaterialScalarParameterInfo>& GetUniformScalarParameters()
	{		
		return this->UniformScalarParameters;
	}
	TMemoryImageArray<FMaterialVectorParameterInfo>& GetUniformVectorParameters()
	{		
		return this->UniformVectorParameters;
	}
	TMemoryImageArray<FMaterialUniformPreshaderHeader>& GetVectorPreshaders()
	{
		return this->UniformVectorPreshaders;
	}
	TMemoryImageArray<FMaterialUniformPreshaderHeader>& GetScalarPreshaders()
	{
		return this->UniformScalarPreshaders;
	}
	FMaterialPreshaderData& GetUniformPreshaderData()
	{
		return UniformPreshaderData;
	}
};

class ShaderProperty
{
public:
	std::string Name;
	int ScalarIndex = -1;
	int VectorIndex = -1;
	int BufferOffset= -1;
};
#if ENGINE_MINOR_VERSION >= 26
using FPreshaderStack = TArray<FLinearColor, TInlineAllocator<64u>>;
void EvaluatePreshader2( const FUniformExpressionSet* UniformExpressionSet, const FMaterialRenderContext& Context, FPreshaderStack& Stack, FPreshaderDataContext& RESTRICT Data, FLinearColor& OutValue,
						 uint16& LastVectorIndex, uint16& LastScalarIndex, int& NumOpCodes );
void GetPropertyNames( FUniformExpressionSet_Override* Set_Override, FMaterialRenderContext MaterialRenderContext, std::vector< ShaderProperty>& Properties, uint8* TempBuffer, int TempBufferSize )
{
	void* BufferCursor = TempBuffer;
	TMemoryImageArray<FMaterialUniformPreshaderHeader>& VectorPreshaders = Set_Override->GetVectorPreshaders();
	TMemoryImageArray<FMaterialUniformPreshaderHeader>& ScalarPreshaders = Set_Override->GetScalarPreshaders();
	TMemoryImageArray<FMaterialScalarParameterInfo>& UniformScalarParameters = Set_Override->GetUniformScalarParameters();
	TMemoryImageArray<FMaterialVectorParameterInfo>& UniformVectorParameters = Set_Override->GetUniformVectorParameters();

	FPreshaderStack PreshaderStack;
	FPreshaderDataContext PreshaderBaseContext( Set_Override->GetUniformPreshaderData() );
	for( int32 VectorIndex = 0; VectorIndex < VectorPreshaders.Num(); ++VectorIndex )
	{
		FLinearColor VectorValue( 0, 0, 0, 0 );

		const FMaterialUniformPreshaderHeader& Preshader = VectorPreshaders[ VectorIndex ];
		FPreshaderDataContext PreshaderContext( PreshaderBaseContext, Preshader );
		uint16 LastVectorIndex;
		uint16 LastScalarIndex;
		int NumOpCodes;
		EvaluatePreshader2( Set_Override, MaterialRenderContext, PreshaderStack, PreshaderContext, VectorValue, LastVectorIndex, LastScalarIndex, NumOpCodes );
		if( NumOpCodes == 1 && LastVectorIndex != (uint16)-1 )
		{
			ShaderProperty NewShaderProperty;
			const FMaterialVectorParameterInfo& Parameter = Set_Override->GetVectorParameter( LastVectorIndex );
			FString UStr = Parameter.ParameterInfo.Name.ToString();
			const wchar_t* WideStr = *UStr;
			NewShaderProperty.Name = FromWideString( WideStr );
			NewShaderProperty.VectorIndex = LastVectorIndex;
			NewShaderProperty.BufferOffset = VectorIndex;
			Properties.push_back( NewShaderProperty );
		}

		FLinearColor* DestAddress = (FLinearColor*)BufferCursor;
		*DestAddress = VectorValue;
		BufferCursor = DestAddress + 1;
		check( BufferCursor <= TempBuffer + TempBufferSize );
	}

	// Dump scalar expression into the buffer.
	for( int32 ScalarIndex = 0; ScalarIndex < ScalarPreshaders.Num(); ++ScalarIndex )
	{
		FLinearColor VectorValue( 0, 0, 0, 0 );

		const FMaterialUniformPreshaderHeader& Preshader = ScalarPreshaders[ ScalarIndex ];
		FPreshaderDataContext PreshaderContext( PreshaderBaseContext, Preshader );
		uint16 LastVectorIndex;
		uint16 LastScalarIndex;
		int NumOpCodes;
		EvaluatePreshader2( Set_Override, MaterialRenderContext, PreshaderStack, PreshaderContext, VectorValue, LastVectorIndex, LastScalarIndex, NumOpCodes );
		if( NumOpCodes == 1 && LastScalarIndex != (uint16)-1 )
		{
			ShaderProperty NewShaderProperty;
			const FMaterialScalarParameterInfo& Parameter = Set_Override->GetScalarParameter( LastScalarIndex );
			FString UStr = Parameter.ParameterInfo.Name.ToString();
			const wchar_t* WideStr = *UStr;
			NewShaderProperty.Name = FromWideString( WideStr );
			NewShaderProperty.ScalarIndex = LastScalarIndex;
			NewShaderProperty.BufferOffset = ScalarIndex;
			Properties.push_back( NewShaderProperty );
		}
		float* DestAddress = (float*)BufferCursor;
		*DestAddress = VectorValue.R;
		BufferCursor = DestAddress + 1;
		check( BufferCursor <= TempBuffer + TempBufferSize );
	}
}
#endif
std::string GenerateInitializeExpressions( MaterialBinding *Mat, int & NumVectorExpressions, int & NumScalarExpressions )
{
	std::string DataString;

	if( !Mat || !Mat->MaterialResource )
	{
		UE_LOG( LogTemp, Error, TEXT( "ERROR! !Mat || !Mat->MaterialResource" ) );
		GLog->Flush();
		return "";
	}

	FMaterialResource* MaterialResource = Mat->MaterialResource;// M->GetMaterialResource( ERHIFeatureLevel::SM5 );
	const FMaterialShaderMap* ShaderMapToUse = MaterialResource->GetRenderingThreadShaderMap();
	
	if( !ShaderMapToUse )
	{
		UE_LOG( LogTemp, Error, TEXT( "ERROR! !ShaderMapToUse" ) );
		GLog->Flush();
		return "";
	}
	
	const FUniformExpressionSet* Set = &ShaderMapToUse->GetUniformExpressionSet();
	if( !Set )
	{
		UE_LOG( LogTemp, Error, TEXT( "ERROR! !Set" ) );
		GLog->Flush();
		return "";
	}
	FUniformExpressionSet_Override* Set_Override = (FUniformExpressionSet_Override*)Set;

	TMemoryImageArray<FMaterialUniformPreshaderHeader>& VectorPreshaders = Set_Override->GetVectorPreshaders();
	TMemoryImageArray<FMaterialUniformPreshaderHeader>& ScalarPreshaders = Set_Override->GetScalarPreshaders();

	NumVectorExpressions = VectorPreshaders.Num();
	NumScalarExpressions = ScalarPreshaders.Num();
	int NumScalarVectors = ( NumScalarExpressions + 3 ) / 4;
	
	int ExtraSpaceForResources = 300;
	int TempBufferSize = ( NumVectorExpressions + NumScalarExpressions ) * 16 + ExtraSpaceForResources;
	
	int FinalTempBufferSize = TempBufferSize + ExtraSpaceForResources;
	uint8* TempBuffer = new uint8[ FinalTempBufferSize ];

	//Prevent nans from appearing in ScalarExpressions values
	float* FloatArray = (float*)TempBuffer;
	for( int i = 0; i < FinalTempBufferSize / 4; i++ )
	{
		FloatArray[ i ] = 0.0f;
	}

	FMaterialRenderProxy* RenderProxy = Mat->RenderProxy;
	if( !RenderProxy )
	{
		UE_LOG( LogTemp, Error, TEXT( "ERROR! !RenderProxy" ) );
		GLog->Flush();
		return "";
	}
	auto Cache = RenderProxy->UniformExpressionCache[ (int32)ERHIFeatureLevel::SM5 ];
	if( Cache.AllocatedVTs.Num() > 0 )
	{
		UE_LOG( LogTemp, Error, TEXT( "[UTU] ! Cache.AllocatedVTs.Num() > 0" ) );
		//Fix for crash in FillUniformBuffer
		return "";
	}
	FMaterialRenderContext DummyContext( RenderProxy, *MaterialResource, nullptr );

	std::vector< ShaderProperty > Properties;
#if ENGINE_MINOR_VERSION >= 26
	GetPropertyNames( Set_Override, DummyContext, Properties, TempBuffer, TempBufferSize );
#endif

	Set->FillUniformBuffer( DummyContext, Cache, TempBuffer, TempBufferSize );	

	char ValueStr[ 256 ];
	sprintf_s( ValueStr,
			   "void InitializeExpressions()\r\n"
			   "{\r\n" );// , VectorExpressions.Num(), ScalarExpressions.Num() );
	DataString += ValueStr;

	uint8* BufferCursor = TempBuffer;
	for( int i = 0; i < NumVectorExpressions; i++ )
	{
		FLinearColor OutValue = *(FLinearColor*)BufferCursor;
		BufferCursor += sizeof( FLinearColor );
		sprintf_s( ValueStr, "\tMaterial.VectorExpressions[%d] = float4(%f,%f,%f,%f);//\r\n", i, OutValue.R, OutValue.G, OutValue.B, OutValue.A );
		DataString += ValueStr;
	}

	for( int i = 0; i < NumScalarVectors; i++ )
	{
		FLinearColor ScalarValues = *(FLinearColor*)BufferCursor;
		BufferCursor += sizeof( FLinearColor );

		sprintf_s( ValueStr, "\tMaterial.ScalarExpressions[%d] = float4(%f,%f,%f,%f);\r\n", i,
					ScalarValues.R, ScalarValues.G, ScalarValues.B, ScalarValues.A );
		DataString += ValueStr;
	}	

	DataString += "}\r\n";
	delete[] TempBuffer;
	return DataString;
}

std::string AddNumberInString( std::string OriginalString, const char *Prefix, int Number )
{
	int PrefixOffset = OriginalString.find( Prefix );
	int NumberStart = PrefixOffset + strlen( Prefix );

	char NumberStr[ 32 ];
	sprintf_s( NumberStr, "%d", Number );
	//Erases the existing "1"
	OriginalString.replace( NumberStart, 1, "" );
	OriginalString.insert( NumberStart, NumberStr );
	return OriginalString;
}
void StringReplacement( std::string & OriginalString, const char *What, const char* Replace )
{
	int pos = 0;
	pos = OriginalString.find( What, pos );
	while( pos != -1 )
	{
		OriginalString.replace( pos, strlen( Replace ), Replace );
		pos = OriginalString.find( What, pos + 1 );
	}
}
const int MaxTextures = 32;
void DetectNumTexturesUsed( std::string GeneratedShader, int* Actual2DTexturesUsed, int* ActualCubeTexturesUsed )
{
	for(int i = 0; i < 32; i++ )
	{
		char Tex2DRef[ 256 ];
		sprintf_s( Tex2DRef, "Material_Texture2D_%d", i );
		int pos = GeneratedShader.find( Tex2DRef );
		if( pos != -1 )
		{
			Actual2DTexturesUsed[i] = 1;
			continue;
		}
	}

	for( int i = 0; i < 32; i++ )
	{
		char TexCubeRef[ 256 ];
		sprintf_s( TexCubeRef, "Material_TextureCube_%d", i );
		int pos = GeneratedShader.find( TexCubeRef );
		if( pos != -1 )
		{
			ActualCubeTexturesUsed[ i ] = 1;
		}		
	}
}
void GeneratePropertyFields( std::string & GeneratedShader )
{	
	int Actual2DTexturesUsed[ MaxTextures ] = { 0 };
	int ActualCubeTexturesUsed[ MaxTextures ] = { 0 };
	DetectNumTexturesUsed( GeneratedShader, Actual2DTexturesUsed, ActualCubeTexturesUsed );

	int MainTexStart = GeneratedShader.find( "_MainTex" );
	if( MainTexStart == -1 )
	{
		UE_LOG( LogTemp, Error, TEXT( "ERROR! GeneratePropertyFields pos == -1 wtf ?" ) );
		return;
	}

	int Cursor = GeneratedShader.find( "\n", MainTexStart );
	for( int i = 0; i < MaxTextures; i++ )
	{
		bool Used = ( Actual2DTexturesUsed[ i ] == 1 );
		if( Used )
		{
			char Line[ 1024 ];
			sprintf_s( Line, "\t\tMaterial_Texture2D_%d( \"Tex%d\", 2D ) = \"white\" {}\n", i, i );
			int LineLen = strlen( Line );
			GeneratedShader.insert( Cursor, Line );
			Cursor += LineLen;
		}
	}
	for( int i = 0; i < MaxTextures; i++ )
	{
		bool Used = ( ActualCubeTexturesUsed[ i ] == 1 );
		if( Used )
		{
			char Line[ 1024 ];
			sprintf_s( Line, "\t\tMaterial_TextureCube_%d( \"TexCube%d\", 2D ) = \"white\" {}\n", i, i );
			int LineLen = strlen( Line );
			GeneratedShader.insert( Cursor, Line );
			Cursor += LineLen;
		}
	}

	const char* SamplersStart = "//samplers start";
	Cursor = GeneratedShader.find( SamplersStart, Cursor );
	Cursor = GeneratedShader.find( "\n", Cursor );
	Cursor++;

	for( int i = 0; i < MaxTextures; i++ )
	{
		bool Used = (Actual2DTexturesUsed[ i ] == 1);
		if( Used )
		{
			char Line[ 1024 ];
			sprintf_s( Line, "\t\t\tuniform sampler2D    Material_Texture2D_%d;\n"
					   "\t\t\tuniform SamplerState Material_Texture2D_%dSampler;\n", i, i );
			int LineLen = strlen( Line );
			GeneratedShader.insert( Cursor, Line );
			Cursor += LineLen;
		}
	}
	for( int i = 0; i < MaxTextures; i++ )
	{
		bool Used = ( ActualCubeTexturesUsed[ i ] == 1 );
		if( Used )
		{
			char Line[ 1024 ];
			sprintf_s( Line, "\t\t\tuniform samplerCUBE  Material_TextureCube_%d;\n"
					   "\t\t\tuniform SamplerState Material_TextureCube_%dSampler;\n", i, i );
			int LineLen = strlen( Line );
			GeneratedShader.insert( Cursor, Line );
			Cursor += LineLen;
		}
	}
}

MaterialBinding::MaterialBinding()
{
	UnityMat = new UnityMaterial();
	UnityScene->Materials.push_back( UnityMat );
}
FString MaterialBinding::GenerateName()
{
	FString MatName = MaterialInterface->GetName();
	
	UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>( MaterialInterface );
	if( MID )
	{
		int Index = -1;
		FString ParentName = MID->Parent->GetName();
		FString FinalName = ParentName + L"(" + MatName + L")";
		UnityMat->Name = ToANSIString( *FinalName );
		return FinalName;
	}
	else
	{
		UnityMat->Name = ToANSIString( *MatName );
		return MatName;
	}
}
bool IsTextureNormalMap( UMaterial* Mat, UTexture *Tex )
{
	TArray<UMaterialExpression*> OutExpressions;
	Mat->GetAllReferencedExpressions( OutExpressions, nullptr );
	for( int i = 0; i < OutExpressions.Num(); i++ )
	{
		UMaterialExpression* Exp = OutExpressions[ i ];
		UMaterialExpressionTextureSample* SampleExpression = Cast< UMaterialExpressionTextureSample>( Exp );
		if( SampleExpression )
		{
			if( SampleExpression->Texture == Tex )
			{
				if( SampleExpression->SamplerType == SAMPLERTYPE_Normal )
					return true;
				else
					return false;
				//if( SampleExpression->SamplerType == SAMPLERTYPE_Color )
			}
		}
	}

	return false;
}
bool IsTextureNormalMap( UMaterialInstance* MaterialInstance, UTexture* Tex )
{
	UMaterial* BaseMaterial = MaterialInstance->GetBaseMaterial();
	bool IsNormalMap = false;
	TArray<FMaterialParameterInfo> OutParameterInfo;
	TArray<FGuid> OutParameterIds;
	MaterialInstance->GetAllTextureParameterInfo( OutParameterInfo, OutParameterIds );
	for( int i = 0; i < OutParameterInfo.Num(); i++ )
	{
		FMaterialParameterInfo& Info = OutParameterInfo[ i ];

		UTexture* OutValue = nullptr;
		if( MaterialInstance->GetTextureParameterValue( Info, OutValue ) && OutValue )
		{
			if( OutValue == Tex )
			{
				UTexture* DefaultTexture = nullptr;
				if( BaseMaterial->GetTextureParameterDefaultValue( Info, DefaultTexture ) && DefaultTexture )
				{
					IsNormalMap = IsTextureNormalMap( BaseMaterial, DefaultTexture );
					return IsNormalMap;
				}
			}
		}
	}

	return false;
}
void ExportMaterialTextures( MaterialBinding *Mat )
{
	UMaterialInterface *UnrealMat = Mat->MaterialInterface;

	TArray<UTexture*> UsedTextures;
	TArray< TArray<int32> > TextureIndices;	
	UnrealMat->GetUsedTexturesAndIndices( UsedTextures, TextureIndices, GlobalQualityLevel, GlobalFeatureLevel );

	UMaterial* BaseMaterial = UnrealMat->GetBaseMaterial();
	UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>( UnrealMat );

	for( int i = 0; i < UsedTextures.Num(); i++ )
	{
		UTexture *Tex = UsedTextures[ i ];

		bool IsNormalMap = false;
		if( MaterialInstance )
		{
			IsNormalMap = IsTextureNormalMap( MaterialInstance, Tex);
		}
		else
			IsNormalMap = IsTextureNormalMap( BaseMaterial, Tex );
		
		TextureBinding *Binding = UETextureExporter::Export( Tex );
		Binding->IsNormalMap = IsNormalMap;
		if( !Binding->UnityTex )
		{
			Binding->UnityTex = new UnityTexture;
			std::wstring TexturePath = GenerateTexturePath( Tex );
			Binding->UnityTex->File = ToANSIString( TexturePath.c_str() );
			Binding->UnityTex->GenerateGUID();
		}

		Mat->UnityMat->Textures.push_back( Binding->UnityTex );
	}
}

void FDebugViewModeMaterialProxy_AddShader( UMaterialInterface* InMaterialInterface, EMaterialQualityLevel::Type QualityLevel, ERHIFeatureLevel::Type FeatureLevel, bool bSynchronousCompilation, EMaterialShaderMapUsage::Type InUsage )
{
	if( !InMaterialInterface ) return;

	const FMaterial* Material = InMaterialInterface->GetMaterialResource( FeatureLevel );
	if( !Material ) 
		return;
}

void GetMaterialShaderSource( MaterialBinding *Mat, bool IsOnGamethread )
{
	UMaterialInterface *M = Mat->MaterialInterface;
	bool WantsGameThread = false;
	UMaterialInstance *MatInstance = dynamic_cast<UMaterialInstance*>( M );
	if( MatInstance )
	{
		Mat->IsInstance = true;
		WantsGameThread = true;
		
		if ( IsOnGamethread )
		{
			FString ProjectDir = FPaths::ProjectContentDir();
			ProjectDir += "../";
			
			FString ShaderFile = ProjectDir;
			ShaderFile += L"/Saved/ShaderDebugInfo/PCD3D_SM5/FDebugViewModeMaterialProxy ";
			ShaderFile += M->GetName();
			ShaderFile += L"/FLocalVertexFactory/FMaterialTexCoordScalePS/MaterialTexCoordScalesPixelShader.usf";

			ShaderFile = FPaths::ConvertRelativePathToFull( ShaderFile );

			if ( !FPlatformFileManager::Get().GetPlatformFile().FileExists( *ShaderFile ) || ForceShaderRecompilation )
			{
				FMaterialEditorUtilities::BuildTextureStreamingData( MatInstance );
			}

			std::wstring ShaderFileWStr = *ShaderFile;
			std::string ShaderFileStr = ToANSIString( ShaderFileWStr.c_str() );
			const char *ShaderFile2 = ShaderFileStr.c_str();
			BYTE *Buffer = nullptr;
			int Size = LoadFile( ShaderFile2, &Buffer );
			if ( Size > 0 )
			{
				std::wstring ShaderSourceWStr = ToWideString( ( char* )Buffer );
				const wchar_t *ShaderSource = ShaderSourceWStr.c_str();
				Mat->ShaderSource = ShaderSource;
				delete[] Buffer;
			}
		}		
	}

	if( WantsGameThread == IsOnGamethread )
	{
		Mat->MaterialResource = M->GetMaterialResource( GlobalFeatureLevel );
		Mat->RenderProxy = M->GetRenderProxy( );
		//if (!MatInstance )
			Mat->MaterialResource->GetMaterialExpressionSource( Mat->ShaderSource );

		auto ReferencedTextures = Mat->MaterialResource->GetReferencedTextures();
		ReferencedTextures = ReferencedTextures;
	}
}

std::string GetLine( std::string String, int position )
{
	int LineStart = String.rfind( '\n', position );
	int LineEnd = String.find( '\n', position );
	if( LineStart == LineEnd )
		return "";
	if( LineStart != -1 && LineEnd != -1 )
	{
		return String.substr( LineStart + 1, LineEnd - LineStart - 2 );
	}
	else
		return "";
}
int GetNumTexcoords( std::string UnrealHLSL )
{
	int pos = UnrealHLSL.find( "#define NUM_TEX_COORD_INTERPOLATORS" );
	if( pos != -1 )
	{
		std::string Line = GetLine( UnrealHLSL, pos );
		if( Line.length() > 0 )
		{
			int NumTexcoords = 0;
			if( sscanf_s( Line.c_str(), "#define NUM_TEX_COORD_INTERPOLATORS %d", &NumTexcoords ) == 1 )
			{
				return NumTexcoords;
			}
		}
	}

	return 0;
}

int GetEndBraceOffset( std::string AllCode, int StartPos )
{
	int ExpStart = AllCode.rfind( '\n', StartPos );
	int FirstBrace = AllCode.find( '{', ExpStart );

	int Depth = 0;
	const char* str = AllCode.c_str();
	for( int i = FirstBrace + 1; i < AllCode.length(); i++ )
	{
		if( AllCode[ i ] == '{' )
		{
			Depth++;
		}
		if( AllCode[ i ] == '}' )
		{
			Depth--;
		}
		if( Depth < 0 )
		{			
			return i;
		}
	}

	UE_LOG( LogTemp, Error, TEXT( "ERROR! GetEndBraceOffset didn't found the end of the function, wtf ?" ) );

	return ExpStart;
}
//Convert HLSL Samples to CG
void ReplaceHLSLSampleCall( std::string & Exp, const char* strToFind, const char* ReplacementFunc )
{
	int FindLen = strlen( strToFind );

	int pos = -1;
	do
	{
		pos = Exp.find( strToFind );
		if( pos != -1 )
		{
			std::string TexName;
			int TexBegin = -1;
			for( int i = pos - 1; i > 0; i-- )
			{
				char c = Exp[ i ];
				if( c == ' ' || c == '\t' || c == '\n' || c == ',' || c == '(' || c == ')' || c == '{' || c == '}' )
				{
					TexName = Exp.substr( i + 1, pos - i - 1 );
					TexBegin = i + 1;
					break;
				}
			}
			if( TexBegin == -1 )
			{
				UE_LOG( LogTemp, Error, TEXT( "ERROR! ProcessCustomExpression couldn't detect TexBegin !" ) );
				return;
			}

			std::string Replacement = ReplacementFunc;
			Replacement += TexName;
			Replacement += ", ";
			Exp.replace( TexBegin, TexName.length() + FindLen, Replacement.c_str() );
		}
	} while( pos != -1 );
}
void ProcessCustomExpression( std::string& Exp )
{
	ReplaceHLSLSampleCall( Exp, ".SampleGrad(", "Texture2DSampleGrad( ");
	ReplaceHLSLSampleCall( Exp, ".Sample(", "Texture2DSample( ");
	ReplaceHLSLSampleCall( Exp, ".SampleLevel(", "Texture2DSampleLevel( ");
	ReplaceHLSLSampleCall( Exp, ".SampleBias(", "Texture2DSampleBias( " );

	StringReplacement( Exp, "Material.", "Material_" );
}
void FixCustomExpressions( std::string & MaterialCode, std::string AllCode )
{
	int Index = 0;
	char Text[ 64 ];
	size_t pos = 0;
	do
	{
		sprintf_s( Text, "CustomExpression%d", Index );
		pos = AllCode.find( Text );
		if ( pos != -1 )
		{
			int ExpStart = AllCode.rfind( '\n', pos );
			int EndBrace = GetEndBraceOffset( AllCode, pos );

			std::string ExpFunction = AllCode.substr( ExpStart, EndBrace + 1 - ExpStart );
			ProcessCustomExpression( ExpFunction );
			std::string MaterialCodeWithExpressions = ExpFunction;
			MaterialCodeWithExpressions += '\n';
			MaterialCodeWithExpressions += MaterialCode;
			MaterialCode = MaterialCodeWithExpressions;
		}
		Index++;
	}
	while ( pos != -1 );
}
std::string GetMaterialCollectionString( const UMaterialParameterCollection* Collection, int CollectionIndex)
{
	int NumVectorsForScalars = ( Collection->ScalarParameters.Num() + 3 ) / 4;
	int TotalVectors = NumVectorsForScalars + Collection->VectorParameters.Num();

	FLinearColor* ScalarVectors = new FLinearColor[ NumVectorsForScalars ];
	std::string ValuesStr;
	std::string* ParamNames = new std::string[ NumVectorsForScalars * 4 ];
	for( int i = 0; i < Collection->ScalarParameters.Num(); i++ )
	{
		FCollectionScalarParameter Param = Collection->ScalarParameters[ i ];
		float* FloatArray = (float*)ScalarVectors;
		FloatArray[ i ] = Param.DefaultValue;
		ParamNames[ i ] = ToANSIString( *Param.ParameterName.ToString());
	}
	for( int i = 0; i < NumVectorsForScalars; i++ )
	{
		char Line[ 1024 ] = "";
		sprintf_s( Line, "\tMaterialCollection%d.Vectors[%d] = float4(%f,%f,%f,%f);//%s,%s,%s,%s\n", CollectionIndex, i, ScalarVectors[i].R, ScalarVectors[ i ].G, ScalarVectors[ i ].B, ScalarVectors[ i ].A,
				   ParamNames[ i * 4 + 0].c_str(), ParamNames[ i * 4 + 1 ].c_str(), ParamNames[ i * 4 + 2 ].c_str(), ParamNames[ i * 4 + 3 ].c_str() );
		ValuesStr += Line;
	}
	for( int i = 0; i < Collection->VectorParameters.Num(); i++ )
	{
		FCollectionVectorParameter Param = Collection->VectorParameters[ i ];
		FLinearColor V = Param.DefaultValue;
		char Line[ 1024 ] = "";
		std::string str = ToANSIString( *Param.ParameterName.ToString() );
		sprintf_s( Line, "\tMaterialCollection%d.Vectors[%d] = float4(%f,%f,%f,%f);//%s\n", CollectionIndex, NumVectorsForScalars + i, V.R, V.G, V.B, V.A, str.c_str() );
		ValuesStr += Line;
	}

	char Text[ 1024 ] = "";
	sprintf_s( Text, "struct MaterialCollection%dType\n"
		"{\n"
		"\tfloat4 Vectors[%d];\n"
		"};\n"
		"MaterialCollection%dType MaterialCollection%d;\n"
		"void Initialize_MaterialCollection%d()\n"
		"{\n", CollectionIndex, TotalVectors, CollectionIndex , CollectionIndex , CollectionIndex );
	std::string Ret = Text;
	Ret += ValuesStr;

	Ret += "}\n";

	delete[] ScalarVectors;
	delete[] ParamNames;
	return Ret;
}
void UncommentLine( std::string & ShaderString, const char *Marker )
{	
	size_t Pos = ShaderString.find( Marker );
	ShaderString.replace( Pos - 2, 2 + strlen( Marker ), "" );//uncomment it
}
std::vector<std::string> GetLines( std::string String, int position, int NumLines )
{
	std::vector<std::string> Lines;

	int FirstLineStart = String.rfind( '\n', position );
	int Cursor = FirstLineStart + 1;
	for( int i = 0; i < NumLines; i++ )
	{
		std::string Line = GetLine( String, Cursor );
		Cursor += Line.length() + 2;
		Lines.push_back( Line );
	}

	return Lines;
}
void FindAndAddInterpolator( const char* Identifier, std::string HLSL, std::string& GeneratedShader, int InterpStart, int& NextLineMarker )
{
	int DefineCursor = HLSL.find( Identifier, InterpStart );
	if( DefineCursor != -1 )
	{
		std::string Line = GetLine( HLSL, DefineCursor );
		Line += "\n";
		GeneratedShader.insert( NextLineMarker, Line );
		NextLineMarker += Line.length();
	}
}
void ProcessVertexInterpolators( std::string HLSL, std::string & GeneratedShader )
{
	int NumInterpolators = -1;
	int InterpStart = HLSL.find( "NUM_CUSTOM_VERTEX_INTERPOLATORS" );
	if( InterpStart > 0 )
	{
		std::string Line = GetLine( HLSL, InterpStart );
		if( Line.length() > 0 )
		{			
			sscanf_s( Line.c_str(), "#define NUM_CUSTOM_VERTEX_INTERPOLATORS %d", &NumInterpolators );
		}
	}

	if( NumInterpolators > 0 )
	{
		const char* NUM_CUSTOM_VERTEX_INTERPOLATORS = "NUM_CUSTOM_VERTEX_INTERPOLATORS";
		int NumInterpolatorsCursor = GeneratedShader.find( NUM_CUSTOM_VERTEX_INTERPOLATORS );
		if( NumInterpolatorsCursor == -1 )
			return;

		char Text[ 256 ];
		sprintf_s( Text, "%d", NumInterpolators );
		GeneratedShader.replace( NumInterpolatorsCursor + strlen( NUM_CUSTOM_VERTEX_INTERPOLATORS ) + 1, strlen( Text ), Text );
		int NextLineMarker = GeneratedShader.find( '\n', NumInterpolatorsCursor );
		NextLineMarker++;

		for( int i = 0; i < NumInterpolators; i++ )
		{
			sprintf_s( Text, "#define VERTEX_INTERPOLATOR_%d_TEXCOORDS_X", i );
			FindAndAddInterpolator( Text, HLSL, GeneratedShader, InterpStart, NextLineMarker );
			sprintf_s( Text, "#define VERTEX_INTERPOLATOR_%d_TEXCOORDS_Y", i );
			FindAndAddInterpolator( Text, HLSL, GeneratedShader, InterpStart, NextLineMarker );
			sprintf_s( Text, "#define VERTEX_INTERPOLATOR_%d_TEXCOORDS_Z", i );
			FindAndAddInterpolator( Text, HLSL, GeneratedShader, InterpStart, NextLineMarker );
			sprintf_s( Text, "#define VERTEX_INTERPOLATOR_%d_TEXCOORDS_W", i );
			FindAndAddInterpolator( Text, HLSL, GeneratedShader, InterpStart, NextLineMarker );
		}		
	}
}
void ProcessMaterial_RenderThread( MaterialBinding *Mat, const char *ProxyShaderString )
{
	GetMaterialShaderSource( Mat, false );

	bool UsesAlphaTest = false;
	bool IsTransparent = false;

	auto OpacityMaskExp = Mat->BaseMaterial->OpacityMask.Expression;
	
	auto AttributesExp = Mat->BaseMaterial->MaterialAttributes.Expression;
	if ( AttributesExp )
	{
		UMaterialExpressionMakeMaterialAttributes *MakeMaterialAttributes = dynamic_cast<UMaterialExpressionMakeMaterialAttributes*>( AttributesExp );
		if ( MakeMaterialAttributes )
		{
			OpacityMaskExp = MakeMaterialAttributes->OpacityMask.Expression;
		}		
	}

	//if ( OpacityMaskExp )
	if ( Mat->BaseMaterial->BlendMode == BLEND_Masked )
		UsesAlphaTest = true;
	if( Mat->BaseMaterial->BlendMode == BLEND_Translucent || Mat->BaseMaterial->BlendMode == BLEND_Additive || Mat->BaseMaterial->BlendMode == BLEND_Modulate )
		IsTransparent = true;

	std::string MaterialCollectionDefinitions;
	TArray<UMaterialExpression*> OutExpressions;
	TArray<UMaterialParameterCollection*> Collections;
	Mat->BaseMaterial->GetAllReferencedExpressions( OutExpressions, nullptr);
	for( int i = 0; i < OutExpressions.Num(); i++ )
	{
		UMaterialExpression* Exp = OutExpressions[ i ];

		UMaterialExpressionMaterialFunctionCall* ExpMFC = dynamic_cast<UMaterialExpressionMaterialFunctionCall*>( Exp );
		if( ExpMFC )
		{
			const TArray<UMaterialExpression*>* FunctionExpressions = ExpMFC->MaterialFunction->GetFunctionExpressions();

			for( int u = 0; u < FunctionExpressions->Num(); u++ )
			{
				Exp = ( *FunctionExpressions )[ u ];
				OutExpressions.Add( Exp );
			}
			continue;
		}
		UMaterialExpressionCollectionParameter* ExpCollectionParameter = Cast<UMaterialExpressionCollectionParameter>( Exp );

		if( ExpCollectionParameter )
		{
			if( Collections.Find( ExpCollectionParameter->Collection ) == INDEX_NONE )
			{
				const UMaterialParameterCollection* Collection = ExpCollectionParameter->Collection;
				std::string MatCollectionDef = GetMaterialCollectionString( Collection, Collections.Num() );
				MaterialCollectionDefinitions += MatCollectionDef;
				Collections.Add( ExpCollectionParameter->Collection );
			}
		}
	}

	UMaterialInterface *M = Mat->MaterialInterface;

	if ( Mat->ShaderSource.Len() > 0 )
	{
		//FString MatName = M->GetName();
		//Mat->UnityMat->Name = ToANSIString( *MatName );
		FString MatName = Mat->GenerateName();

		int Len = MatName.Len();
		if ( Len > 0 )
		{
			std::wstring ShaderFileName = GetOutputFile( L"Shaders\\", MatName, L".shader" );
			std::wstring UnrealHLSLFile = GetOutputFile( L"Shaders\\", MatName, L".hlsl" );

			std::wstring ShaderOutDir = GetUnityAssetsFolder();
			ShaderOutDir += L"Shaders\\";
			VerifyOrCreateDirectory( ShaderOutDir.c_str() );

			std::wstring ShaderSourceW = *Mat->ShaderSource;
			std::string ShaderSourceANSI = ToANSIString( ShaderSourceW );
			const char *CSource = ShaderSourceANSI.c_str();

			//Debug original UE HLSL in case of issues
			bool DebugUnrealHLSL = false;
			if ( DebugUnrealHLSL )
				SaveFile( ToANSIString( UnrealHLSLFile.c_str() ).c_str(), ( BYTE* )CSource, strlen( CSource ) );
			const char *ptr = strstr( CSource, "void CalcPixelMaterialInputs" );
			if ( ptr )
			{
				int CropStart = ( int64_t )ptr - ( int64_t )CSource;
				const char *EndPtr = strstr( ptr, "}" );
				if ( !EndPtr )
					return;
				int CropEnd = ( int64_t )EndPtr - ( int64_t )CSource;

				int CropSize = CropEnd - CropStart + 1;
				if ( CropSize < 0 )
				{
					return;
				}
				char *CalcPixelMaterialInputsStr = new char[ CropSize ];
				CalcPixelMaterialInputsStr[ CropSize ] = 0;
				memcpy( CalcPixelMaterialInputsStr, ptr, CropSize );

				std::string CalcPixelMaterialInputsFinal = CalcPixelMaterialInputsStr;

				StringReplacement( CalcPixelMaterialInputsFinal, "Material.Texture", "Material_Texture" );
				StringReplacement( CalcPixelMaterialInputsFinal, "Material.Wrap_WorldGroupSettings", "Material_Wrap_WorldGroupSettings" );
				StringReplacement( CalcPixelMaterialInputsFinal, "Material.Clamp_WorldGroupSettings", "Material_Clamp_WorldGroupSettings" );
				if ( Mat->IsInstance )
				{
					StringReplacement( CalcPixelMaterialInputsFinal, "Material_VectorExpressions", "Material.VectorExpressions" );
					StringReplacement( CalcPixelMaterialInputsFinal, "Material_ScalarExpressions", "Material.ScalarExpressions" );
				}

				size_t pos = CalcPixelMaterialInputsFinal.find( "CustomExpression" );
				if ( pos != -1)
				{
					FixCustomExpressions( CalcPixelMaterialInputsFinal, ShaderSourceANSI );
				}
				
				std::string GeneratedShader = ProxyShaderString;

				const char *InsertMarker = "M_Chair";
				std::string MatNameStr = ToANSIString( *MatName );

				int Pos = GeneratedShader.find( InsertMarker );
				GeneratedShader.replace( Pos, strlen( InsertMarker ), MatNameStr.c_str() );

				int NumTexcoords = GetNumTexcoords( ShaderSourceANSI );
				const char *NumTexcoordsMarker = "NUM_TEX_COORD_INTERPOLATORS";
				char NumTexcoordsStr[ 16 ];
				sprintf_s( NumTexcoordsStr, "%d", NumTexcoords );
				Pos = GeneratedShader.find( NumTexcoordsMarker );
				GeneratedShader.replace( Pos + strlen( NumTexcoordsMarker ) + 1, strlen( NumTexcoordsStr ), NumTexcoordsStr );

				//Need to add alpha here if opacity/opacity mask is used ! + render que transparent
				//#pragma surface surf Standard

				//opacity = blending
				//opacity mask = alpha test
				
				if( Mat->MaterialInterface->IsTwoSided() )
				{
					const char *CullingMarker = "Cull Off";
					Pos = GeneratedShader.find( CullingMarker );
					GeneratedShader.replace( Pos - 2, 2, "" );//uncomment it
				}
				if( IsTransparent )
				{
					UncommentLine( GeneratedShader, "BLEND_ON" );
					UncommentLine( GeneratedShader, "BLEND_ON" );
				}
				else
				{
					UncommentLine( GeneratedShader, "BLEND_OFF" );
					UncommentLine( GeneratedShader, "BLEND_OFF" );
				}

				if ( UsesAlphaTest )
				{
					const char *OpacitMaskMarker = "if( PixelMaterialInputs.OpacityMask";
					Pos = GeneratedShader.find( OpacitMaskMarker );
					GeneratedShader.replace( Pos - 2, 2, "" );//uncomment it
				}

				//move to render thread always!
				int NumVectorExpressions = 0;
				int NumScalarExpressions = 0;
				std::string ExpressionsData = GenerateInitializeExpressions( Mat, NumVectorExpressions, NumScalarExpressions );

				NumVectorExpressions = FMath::Max( NumVectorExpressions, 1 );
				NumScalarExpressions = FMath::Max( NumScalarExpressions, 1 );
				
				int NumScalarVectors = ( NumScalarExpressions + 3 ) / 4;
				GeneratedShader = AddNumberInString( GeneratedShader, "VectorExpressions[", NumVectorExpressions );
				GeneratedShader = AddNumberInString( GeneratedShader, "ScalarExpressions[", NumScalarVectors );

				InsertMarker = "MaterialStruct Material;";
				Pos = GeneratedShader.find( InsertMarker );
				Pos += strlen( InsertMarker );

				GeneratedShader.insert( Pos + 1, ExpressionsData );				
				Pos += ExpressionsData.length();

				if( MaterialCollectionDefinitions.length() > 0 )
				{
					GeneratedShader.insert( Pos, MaterialCollectionDefinitions );
					Pos += MaterialCollectionDefinitions.length();

					InsertMarker = "InitializeExpressions();";
					int InitPos = GeneratedShader.find( InsertMarker );
					InitPos += strlen( InsertMarker );

					std::string MaterialCollectionInits;
					for( int i = 0; i < Collections.Num(); i++ )
					{
						char Text[ 1024 ] = "";
						sprintf_s( Text, "\n\tInitialize_MaterialCollection%d();\n", i );
						MaterialCollectionInits += Text;
					}

					GeneratedShader.insert( InitPos, MaterialCollectionInits );
				}

				GeneratedShader.insert( Pos, CalcPixelMaterialInputsFinal );

				GeneratePropertyFields( GeneratedShader );

				ProcessVertexInterpolators( ShaderSourceANSI, GeneratedShader );

				std::string ShaderFileNameANSI = ToANSIString( ShaderFileName.c_str() );
				SaveFile( ShaderFileNameANSI.c_str(), ( BYTE* )GeneratedShader.c_str(), GeneratedShader.length() );
				Mat->UnityMat->ShaderFileName = ShaderFileNameANSI;

				Mat->UnityMat->GenerateShaderGUID();
				std::string ShaderMetaContents = GenerateShaderMeta( Mat->UnityMat->ShaderGUID );
				std::string ShaderMetaFile = ShaderFileNameANSI;
				ShaderMetaFile += ".meta";
				SaveFile( ShaderMetaFile.c_str(), (BYTE*)ShaderMetaContents.c_str(), ShaderMetaContents.length() );

				
				Mat->UnityMat->ShaderContents = GeneratedShader;
			}
		}		
	}
	else
	{
		//UE_LOG( LogEngine, Error, TEXT( "No Shader code ???" ) );
	}	
}

class MeshBinding
{
public:
	UnityMesh *TheUnityMesh = nullptr;
	UStaticMesh *UnrealStaticMesh = nullptr;
};

std::vector< MeshBinding* > MeshList;

UnityMesh * GetUnityMesh( const UStaticMesh *UnrealStaticMesh, const char *Name )
{
	for( int i = 0; i < MeshList.size(); i++ )
	{
		if( MeshList[ i ]->UnrealStaticMesh == UnrealStaticMesh )
			return MeshList[ i ]->TheUnityMesh;
	}

	MeshBinding *NewMeshBinding = new MeshBinding;
	NewMeshBinding->TheUnityMesh = new UnityMesh;
	NewMeshBinding->TheUnityMesh->Name = Name;
	NewMeshBinding->UnrealStaticMesh = ( UStaticMesh*)UnrealStaticMesh;

	MeshList.push_back( NewMeshBinding );
	return NewMeshBinding->TheUnityMesh;
}
void WriteOBJIndices( FILE *f, int NumIndexSets, UINT I1, UINT I2, UINT I3 )
{	
	if( NumIndexSets == 1 )
		fprintf( f, "f %d %d %d\n", I1, I2, I3 );
	else if( NumIndexSets == 2 )
		fprintf( f, "f %d/%d %d/%d %d/%d\n", I1, I1, I2, I2, I3, I3 );
	else if( NumIndexSets == 3 )
		fprintf( f, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", I1, I1, I1, I2, I2, I2, I3, I3, I3 );
}
void ExportOBJ( const char *Filename, UnityMesh* M)
{
	int NumVerts = M->NumVertices;
	FVector* Vertices = M->Vertices;
	FVector* Normals = M->Normals;
	FVector* Tangents = M->Tangents;
	FVector2D* Texcoords = M->Texcoords;
	FVector2D* Texcoords1 = M->Texcoords1;
	FColor* Colors = M->Colors;
	int NumIndices = M->NumIndices;
	UINT* Indices = M->AllIndices;
	const char* MeshName = M->Name.c_str();
	std::vector<MeshSection*>* Sections = &M->Sections;

	FILE *f = nullptr;
	fopen_s( &f, Filename, "w" );
	if( !f )
		return;

	std::string MTLFile = MeshName;
	MTLFile += ".mtl";

	fprintf( f, "mtllib %s\n", MTLFile.c_str() );

	//if ( Sections && Sections->size() > 1 )
		//fprintf( f, "g %s.0\n", MeshName );
	//else
		fprintf( f, "g %s\n", MeshName );

	fprintf( f, "#Vertices %d\n", NumVerts );
	fprintf( f, "#Indices %d\n", NumIndices );
	
	for( int s = 0; s < Sections->size(); s++ )
	{
		auto Section = ( *Sections )[ s ];
		const char* MatStr = "None";
		if( Section->MaterialIndex >= 0 && Section->MaterialIndex < M->Materials.size() )
		{
			UnityMaterial* Mat = M->Materials[ Section->MaterialIndex ];
			if ( Mat )
				MatStr = Mat->Name.c_str();
		}
		fprintf( f, "#Section %d Offset %d NumIndices %d MaterialIndex %d %s\n", s, Section->IndexOffset,Section->NumIndices, Section->MaterialIndex, MatStr );
	}

	for( int i = 0; i < NumVerts; i++ )
	{
		FVector *V = &Vertices[ i ];
		
		if ( Colors )
		{
			FColor *C = &Colors[ i ];
			float RGBA[ 4 ] = { ( ( float )C->R ) / 255.0f, ( ( float )C->G ) / 255.0f, ( ( float )C->B ) / 255.0f, ( ( float )C->A ) / 255.0f };
			fprintf( f, "v %f %f %f %f %f %f %f\n", V->X, V->Y, V->Z, RGBA[ 0 ], RGBA[ 1 ], RGBA[ 2 ], RGBA[ 3 ] );
		}
		else
			fprintf( f, "v %f %f %f\n", V->X, V->Y, V->Z );
	}

	//fprintf( f, "\n" );

	int NumIndexSets = 1;
	if( Texcoords )
	{
		NumIndexSets++;
		for( int i = 0; i < NumVerts; i++ )
		{
			FVector2D *VT = &Texcoords[ i ];
			fprintf( f, "vt %f %f\n", VT->X, VT->Y );
		}
	}
	if( Texcoords1 )
	{
		for( int i = 0; i < NumVerts; i++ )
		{
			FVector2D* VT = &Texcoords1[ i ];
			fprintf( f, "vt1 %f %f\n", VT->X, VT->Y );
		}
	}

	if( Normals )
	{
		NumIndexSets++;
		for( int i = 0; i < NumVerts; i++ )
		{
			FVector *V = &Normals[ i ];
			fprintf( f, "vn %f %f %f\n", V->X, V->Y, V->Z );
		}
	}
	if( Tangents )
	{
		for( int i = 0; i < NumVerts; i++ )
		{
			FVector* T = &Tangents[ i ];
			fprintf( f, "tan %f %f %f %f\n", T->X, T->Y, T->Z, 1.0f );
		}
	}
	if( Indices )
	{
		bool FlipWinding = true;
		if( Sections && Sections->size() > 1 )
		{
			for( int s = 0; s < Sections->size(); s++ )
			{
				auto Section = (*Sections)[ s ];
				if( s > 0 )
				{
					//fprintf( f, "g %s.%d\n", MeshName, s );
				}

				for( UINT i = Section->IndexOffset; i < Section->IndexOffset + Section->NumIndices; i += 3 )
				{
					UINT I1 = Indices[ i + 0 ] + 1;
					UINT I2 = Indices[ i + 1 ] + 1;
					UINT I3 = Indices[ i + 2 ] + 1;

					if( FlipWinding )
					{
						I1 = Indices[ i + 2 ] + 1;
						I2 = Indices[ i + 1 ] + 1;
						I3 = Indices[ i + 0 ] + 1;
					}

					WriteOBJIndices( f, NumIndexSets, I1, I2, I3 );
				}
			}
		}
		else
		{
			for( int i = 0; i < NumIndices; i += 3 )
			{
				UINT I1 = Indices[ i + 0 ] + 1;
				UINT I2 = Indices[ i + 1 ] + 1;
				UINT I3 = Indices[ i + 2 ] + 1;

				if( FlipWinding )
				{
					I1 = Indices[ i + 2 ] + 1;
					I2 = Indices[ i + 1 ] + 1;
					I3 = Indices[ i + 0 ] + 1;
				}

				WriteOBJIndices( f, NumIndexSets, I1, I2, I3 );
			}
		}
	}

	fclose( f );
}

void ProcessSkeletalMeshMaterials( USkeletalMeshComponent *SkeletalMeshComponent )
{
	TArray<UMaterialInterface*> UsedMaterials;
	SkeletalMeshComponent->GetUsedMaterials( UsedMaterials );
	MaterialBinding **MaterialsUsed = nullptr;
	TArray<UMaterialInterface*> Materials = SkeletalMeshComponent->GetMaterials();

	int MaxMaterials = FMath::Max( UsedMaterials.Num(), Materials.Num() );
	MaterialsUsed = new MaterialBinding*[ MaxMaterials ];
	for( int i = 0; i < MaxMaterials; i++ )
		MaterialsUsed[ i ] = nullptr;

	if( UsedMaterials.Num() > 0 )
	{
		for( int i = 0; i < UsedMaterials.Num(); i++ )
		{
			MaterialsUsed[ i ] = nullptr;
			if( !UsedMaterials[ i ] )
				continue;
			MaterialsUsed[ i ] = ProcessMaterialReference( UsedMaterials[ i ] );
		}
	}

	//Apparently used materials is sometimes less than StaticMats, assuming it fills with static mats ?
	if( Materials.Num() > 0 )
	{
		for( int i = UsedMaterials.Num(); i < Materials.Num(); i++ )
		{
			MaterialsUsed[ i ] = nullptr;
			UMaterialInterface *MaterialInterface = Materials[ i ];
			if( !MaterialInterface )
				continue;

			MaterialsUsed[ i ] = ProcessMaterialReference( MaterialInterface );
		}
	}
}
void MoveComponentsToNewGO( GameObject* GO )
{
	GameObject* NewGameObject = new GameObject;
	
	for( int i = 0; i < GO->Components.size(); i++ )
	{
		Component* Comp = GO->Components[ i ];
		
		{
			NewGameObject->Components.push_back( Comp );
			Comp->Owner = NewGameObject;

			GO->Components.erase( GO->Components.begin() + i );
			i--;
		}
	}
	
	{
		MeshRenderer* MR = ( MeshRenderer * )NewGameObject->GetComponent( CT_MESHRENDERER );
		if ( MR )
			NewGameObject->Name = MR->Name;

		GO->AddChild( NewGameObject );
		GO->OwnerScene->Add( NewGameObject );
	}	
}
void ProcessStaticMesh( AActor *A, UStaticMeshComponent *StaticMeshComp, GameObject *GO)
{
	const UStaticMesh * Mesh = StaticMeshComp->GetStaticMesh();
	if( !Mesh )
		return;

	TArray<UMaterialInterface*> OutMaterials;
	StaticMeshComp->GetUsedMaterials( OutMaterials );
	MaterialBinding **MaterialsUsed = nullptr;
	const TArray<FStaticMaterial> StaticMats = Mesh->StaticMaterials;

	int MaxMaterials = FMath::Max( OutMaterials.Num(), StaticMats.Num() );
	MaterialsUsed = new MaterialBinding*[ MaxMaterials ];
	for( int i = 0; i < MaxMaterials; i++ )
		MaterialsUsed[ i ] = nullptr;

	if( OutMaterials.Num() > 0 )
	{
		for( int i = 0; i < OutMaterials.Num(); i++ )
		{			
			MaterialsUsed[ i ] = nullptr;
			if( !OutMaterials[ i ] )
				continue;
			MaterialsUsed[ i ] = ProcessMaterialReference( OutMaterials[ i ] );
		}
	}
	
	//Apparently used materials is sometimes less than StaticMats, assuming it fills with static mats ?
	if( StaticMats.Num() > 0 )
	{
		for( int i = OutMaterials.Num(); i < StaticMats.Num(); i++ )
		{
			MaterialsUsed[ i ] = nullptr;
			UMaterialInterface *MaterialInterface = StaticMats[ i ].MaterialInterface;
			if( !MaterialInterface )
				continue;

			MaterialsUsed[ i ] = ProcessMaterialReference( MaterialInterface );
		}
	}

	if( !ExportMeshes )
		return;

	FStaticMeshRenderData & MeshRenderData = *Mesh->RenderData;
	if( MeshRenderData.LODResources.Num() > 0 )
	{
		FStaticMeshLODResources & LODResources = MeshRenderData.LODResources[ 0 ];

		FString SourceFile = Mesh->GetName();
		if( SourceFile.Len() == 0 )
		{
			SourceFile = A->GetName();
		}
		char Name[ 1024 ] = "Default";
		const wchar_t *SourceFileW = *SourceFile;
		FromUnicode( SourceFileW, Name );

		UnityMesh *NewMesh = GetUnityMesh( Mesh, Name );
		if ( NewMesh->Sections.size() == 0 )
		{
			for ( int i = 0; i < LODResources.Sections.Num(); i++ )
			{
				FStaticMeshSection Section = LODResources.Sections[ i ];
				MeshSection *NewMeshSection = new MeshSection;
				NewMeshSection->MaterialIndex = Section.MaterialIndex;
				NewMeshSection->IndexOffset = Section.FirstIndex;
				NewMeshSection->NumIndices = Section.NumTriangles * 3;

				NewMesh->Sections.push_back( NewMeshSection );
			}


			int NumVertices = LODResources.VertexBuffers.PositionVertexBuffer.GetNumVertices();
			int NumTexcoordChannels = LODResources.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();
			int NumIndices = LODResources.IndexBuffer.GetNumIndices();

			UINT *Indices = new UINT[ NumIndices ];
			FVector *Vertices	 = new FVector[ NumVertices ];
			FVector* Normals	 = new FVector[ NumVertices ];
			FVector* Tangents	 = new FVector[ NumVertices ];
			FVector2D *Texcoords = new FVector2D[ NumVertices ];
			FVector2D* Texcoords1 = nullptr;
			if ( NumTexcoordChannels > 1 )
				Texcoords1 = new FVector2D[ NumVertices ];
			FColor *Colors = nullptr;

			if ( NumIndices > 0 )
			{
				FIndexArrayView ArrayView = LODResources.IndexBuffer.GetArrayView();

				for ( int i = 0; i < NumIndices; i++ )
				{
					//uint16 Index16 = (uint16)ArrayView[ i ];
					uint32 Index32 = ArrayView[ i ];
					Indices[ i ] = Index32;

					if ( LODResources.IndexBuffer.Is32Bit() )
					{

					}
					//else
				}

				TotalTriangles += NumIndices / 3;
			}
			else
				TotalTriangles += NumVertices / 3;

			for ( int i = 0; i < LODResources.Sections.Num(); i++ )
			{
				FStaticMeshSection Section = LODResources.Sections[ i ];
				int MatIndex = Section.MaterialIndex;
				int VertexStart = Section.MinVertexIndex;
				int VertexEnd = Section.MaxVertexIndex;
				int IndexStart = Section.FirstIndex;
				int NumTriangles = Section.NumTriangles;
			}

			TArray<FColor> ColorArray;
			TArray<FPaintedVertex> PaintedVertices;
			
			if( StaticMeshComp->LODData.Num() > 0 )
			{
				if( StaticMeshComp->LODData[ 0 ].OverrideVertexColors )
				{
					StaticMeshComp->LODData[ 0 ].OverrideVertexColors->GetVertexColors( ColorArray );
				}
			}
			
			if( ColorArray.Num() == 0 )
			{
				LODResources.VertexBuffers.ColorVertexBuffer.GetVertexColors( ColorArray );
			}
			if ( ColorArray.Num() > 0 && ColorArray.Num() == NumVertices )
			{
				Colors = new FColor[ ColorArray.Num() ];
			}
			for ( int i = 0; i < NumVertices; i++ )
			{
				FVector Pos = LODResources.VertexBuffers.PositionVertexBuffer.VertexPosition( i );
				//Scale down from Unreal's cm units to Unity's m
				Pos /= 100.0f;

				//if( SwitchCoordinates )
				{
					//std::swap( Pos.Y, Pos.Z );
					//Pos.X *= -1;
				}

				Vertices[ i ] = Pos;

				FVector TangentX = LODResources.VertexBuffers.StaticMeshVertexBuffer.VertexTangentX( i );//tangent
				FVector TangentZ = LODResources.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ( i );//Normal

				Normals[ i ] = TangentZ;
				Tangents[ i ] = TangentX;
				
				FVector2D UV = LODResources.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV( i, 0 );
				Texcoords[ i ] = FVector2D( UV.X, UV.Y );

				if( NumTexcoordChannels > 1 )
				{
					FVector2D UV1 = LODResources.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV( i, 1 );
					Texcoords1[ i ] = FVector2D( UV1.X, UV1.Y );
				}

				if ( Colors )
				{
					Colors[ i ] = ColorArray[ i ];
				}
			}

			NewMesh->Vertices = Vertices;
			NewMesh->Normals = Normals;
			NewMesh->Tangents = Tangents;
			NewMesh->Texcoords = Texcoords;
			NewMesh->Texcoords1 = Texcoords1;
			NewMesh->Colors = Colors;
			NewMesh->AllIndices = Indices;
			NewMesh->NumVertices = NumVertices;
			NewMesh->NumIndices = NumIndices;

			for( int i = 0; i < MaxMaterials; i++ )
			{
				if ( MaterialsUsed[ i ] )
					NewMesh->Materials.push_back( MaterialsUsed[ i ]->UnityMat );
				else
					NewMesh->Materials.push_back( nullptr );
			}
		}
		else
		{

		}
		
		const char* CompName = FromWideString( *StaticMeshComp->GetName() );

		MeshRenderer* MR = nullptr;
		MeshFilter* MF = nullptr; 

		//If there's more mesh components make more gameobjects, in unity only one mesh renderer can be on a gameobject
		{
			//MoveComponentsToNewGO( GO );
			TransformComponent* BaseTransform = (TransformComponent*)GO->GetComponent( CT_TRANSFORM );
			BaseTransform->Reset();
		}

		GameObject* ChildGO = new GameObject;
		GO->AddChild( ChildGO );
		GO->OwnerScene->Add( ChildGO );
								
		TransformComponent* ComponentTransform = (TransformComponent*)ChildGO->GetComponent( CT_TRANSFORM );
		//FTransform RelativeTransform = StaticMeshComp->GetRelativeTransform();
		FTransform ComponentTrans = StaticMeshComp->GetComponentTransform();
		ConvertTransform( ComponentTrans, ComponentTransform, false );// true );

		MR = (MeshRenderer*)ChildGO->AddComponent( CT_MESHRENDERER );
		MF = (MeshFilter*)  ChildGO->AddComponent( CT_MESHFILTER );

		ChildGO->Name = CompName;

		GO = ChildGO;		

		MF->Mesh = NewMesh;
			
		MR->Name = CompName;
		MR->CastShadows = StaticMeshComp->CastShadow;

		//Add materials from this StaticMeshComponent
		for( int i = 0; i < NewMesh->Materials.size(); i++ )
		{
			if( MaterialsUsed[ i ] )
				MR->Materials.push_back( MaterialsUsed[ i ]->UnityMat );
			else
				MR->Materials.push_back( nullptr );
		}
		
	}
}

FVector RotateAroundPoint( FVector Position, FVector Center, float X, float Y, float Z )
{
	FVector Temp;
	Temp.X = Position.X - Center.X;
	Temp.Y = Position.Y - Center.Y;
	Temp.Z = Position.Z - Center.Z;
	if( X )
	{
		Position.Z = (float)( Center.Z + sin( X ) * Temp.Y + cos( X ) * Temp.Z );
		Position.Y = (float)( Center.Y + cos( X ) * Temp.Y - sin( X ) * Temp.Z );
	}
	if( Y )
	{
		Position.Z = (float)( Center.Z + sin( Y ) * Temp.X + cos( Y ) * Temp.Z );
		Position.X = (float)( Center.X + cos( Y ) * Temp.X - sin( Y ) * Temp.Z );
	}
	if( Z )
	{
		Position.X = (float)( Center.X + sin( Z ) * Temp.Y + cos( Z ) * Temp.X );
		Position.Y = (float)( Center.Y + cos( Z ) * Temp.Y - sin( Z ) * Temp.X );
	}

	return Position;
}

UnityLightType GetUnityLightComponentType( ELightComponentType Type )
{
	switch( Type )
	{
		case LightType_Directional:return LT_DIRECTIONAL;
		default:
		case LightType_Point:return LT_POINT;
		case LightType_Spot: return LT_SPOT;
		case LightType_Rect: return LT_AREA;
	}
}
float ConvertSpotAngle( float OuterConeAngle )
{
	float MaxUEAngle = 80;
	float MaxUnityAngle = 179;

	float AngleRatio = OuterConeAngle / MaxUEAngle;
	return AngleRatio * MaxUnityAngle;
}
void ConvertTransform( FTransform Trans, TransformComponent* Transform, bool IsRelative )
{
	FVector OriginalTranslation = Trans.GetTranslation();
	OriginalTranslation /= 100.0f;

	FVector Translation = OriginalTranslation;

	FQuat Quat = Trans.GetRotation();
	FVector Scale = Trans.GetScale3D();

	FVector Euler = Quat.Euler();
	FVector EulerAfter = Euler;

	if (!IsRelative )
	{
		Scale.X *= -1;
	}

	Transform->LocalPosition[ 0 ] = Translation.X;
	Transform->LocalPosition[ 1 ] = Translation.Y;
	Transform->LocalPosition[ 2 ] = Translation.Z;

	Transform->LocalRotation[ 0 ] = Quat.X;
	Transform->LocalRotation[ 1 ] = Quat.Y;
	Transform->LocalRotation[ 2 ] = Quat.Z;
	Transform->LocalRotation[ 3 ] = Quat.W;

	Transform->LocalScale[ 0 ] = Scale.X;
	Transform->LocalScale[ 1 ] = Scale.Y;
	Transform->LocalScale[ 2 ] = Scale.Z;

	if( IsRelative )
	{
		Transform->LocalPosition[ 0 ] *= -1;
	}
}
void ProcessActor( AActor *A, Scene *S)
{
	if ( !A )
		return;

	TArray<UActorComponent*> comps;
	A->GetComponents( comps );

	FTransform Trans = A->GetTransform();	

	const char *ActorName = FromWideString( *A->GetName() );
	
	GameObject *GO = new GameObject();
	S->Add( GO );

	GO->Name = ActorName;
	
	FQuat Quat = Trans.GetRotation();
	TransformComponent* Transform = (TransformComponent*)GO->GetComponent( CT_TRANSFORM );

	ConvertTransform( Trans, Transform );
	
	TArray<FString> MeshCompNames;
	for ( int i = 0; i < comps.Num(); i++ )
	{
		UActorComponent *AC = comps[ i ];
		ULightComponent *LightComp = dynamic_cast<ULightComponent*>( AC );
		if ( LightComp )
		{
			LightComponent* NewLightComponent = new LightComponent;
			TransformComponent* LightTransform = Transform;
			if( comps.Num() > 1 )
			{
				Transform->Reset();
				GameObject* ChildGO = new GameObject;
				ChildGO->Name = FromWideString( *LightComp->GetName() );
				ChildGO->Components.push_back( NewLightComponent );
				NewLightComponent->Owner = ChildGO;
				LightTransform = (TransformComponent*)ChildGO->GetComponent( CT_TRANSFORM );
				
				FTransform ComponentTrans = LightComp->GetComponentTransform();
				ConvertTransform( ComponentTrans, LightTransform, false );

				GO->AddChild( ChildGO );
				GO->OwnerScene->Add( ChildGO );
			}
			else
			{
				NewLightComponent->Owner = GO;
				GO->Components.push_back( NewLightComponent );
			}

			FQuat FixQuat( FVector( 0, 1, 0 ), PI / 2 );
			FQuat FinalQuat = Quat * FixQuat;
			LightTransform->LocalRotation[ 0 ] = FinalQuat.X;
			LightTransform->LocalRotation[ 1 ] = FinalQuat.Y;
			LightTransform->LocalRotation[ 2 ] = FinalQuat.Z;
			LightTransform->LocalRotation[ 3 ] = FinalQuat.W;

			ELightComponentType Type = LightComp->GetLightType();
			if( Type == LightType_Directional )
			{
				UDirectionalLightComponent* DirectionalLightComp = Cast< UDirectionalLightComponent>( LightComp );

				float Brightness = DirectionalLightComp->ComputeLightBrightness();
				NewLightComponent->Intensity = 1.0f;// Brightness / PI;
			}
			else if( Type == LightType_Point )
			{
				UPointLightComponent* PointLightComp = Cast< UPointLightComponent>( LightComp );
				NewLightComponent->Range = PointLightComp->AttenuationRadius / 100.0f;
				
				float Brightness = PointLightComp->ComputeLightBrightness() / 16.0f;
				NewLightComponent->Intensity = 1.0f;// Brightness / 10000.0f;
			}
			else if( Type == LightType_Spot )
			{
				USpotLightComponent* SpotLightComp = Cast< USpotLightComponent>( LightComp );
				NewLightComponent->Range = SpotLightComp->AttenuationRadius / 100.0f;
				NewLightComponent->SpotAngle = ConvertSpotAngle( SpotLightComp->OuterConeAngle );

				float Brightness = SpotLightComp->ComputeLightBrightness();
				NewLightComponent->Intensity = Brightness / 10000.0f;
				NewLightComponent->Intensity = 1.0f;// /= 1.6f;
			}
			else
				NewLightComponent->Intensity = LightComp->Intensity;

			NewLightComponent->Color = LightComp->LightColor;
			NewLightComponent->Type = GetUnityLightComponentType( Type );			
			NewLightComponent->Shadows = LightComp->CastShadows;			
			NewLightComponent->Enabled = LightComp->bAffectsWorld;
			
		}
		USkeletalMeshComponent *SkelMeshComp = dynamic_cast<USkeletalMeshComponent*>( AC );		
		if ( SkelMeshComp )
		{
			ProcessSkeletalMeshMaterials( SkelMeshComp );
		}
		UStaticMeshComponent *StaticMeshComp = dynamic_cast<UStaticMeshComponent*>( AC );
		if ( StaticMeshComp )
		{
			UClass *C = StaticMeshComp->GetClass();
			
			FString CompName = C->GetName();
			MeshCompNames.Add( CompName );

			FString Name;
			Name = A->GetName();			

			ProcessStaticMesh( A, StaticMeshComp, GO );
			
		}
	}	
}
bool VerifyOrCreateDirectory( const FString& TestDir )
{
	const wchar_t *WChar = *TestDir;
	return VerifyOrCreateDirectory( WChar );
}
bool VerifyOrCreateDirectory( const wchar_t * TestDir )
{
	//Directory Exists?
	if ( !FPlatformFileManager::Get().GetPlatformFile().DirectoryExists( TestDir ) )
	{
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectory( TestDir );

		if ( !FPlatformFileManager::Get().GetPlatformFile().DirectoryExists( TestDir ) )
		{
			return false;
		}
	}
	return true;
}
bool SaveFile( const char *File, void *data, int64_t size )
{
	FILE *f = nullptr;
	if( File && strcmp( File, "" ) != 0 )
		fopen_s(&f, File, "wb" );

	if( !f )
		return false;

	fwrite( data, sizeof( char ), (int)size, f );
	fclose( f );
	return true;
}
int64_t GetFileSize( const char *FileName )
{
	if( !FileName || !FileName[ 0 ] )
		return -1;
	int64_t size = 0;
	FILE* f = nullptr;
	fopen_s( &f, FileName, "rb" );
	if( !f )
		return -1;

	fseek( f, 0, SEEK_END );
	size = ftell( f );

	if( size == -1 )
	{
		//RELog( "The file may be over 2GB, use _ftelli64 !" );
		size = _ftelli64( f );

	}
	fclose( f );
	return size;
}
int64_t LoadFile( const char *File, BYTE **data )
{
	if( !File || !data || File[ 0 ] == 0 )
		return -1;

	int64_t size = 0;
	size = GetFileSize( File );

	FILE *f = nullptr;
	fopen_s( &f, File, "rb" );
	if( !f )
	{
		int Ret = -1;	
		return Ret;
	}

	//Add '0' character (if presumed to be acting like a string)
	//might impply allocation errors ?
	*data = new BYTE[ (size_t)size + 1 ];
	if( !( *data ) )
	{
		//RELog( "LoadFile Allocation Error" );
		return -1;
	}
	( *data )[ size ] = 0;//last character should be 0 in case it's a string

	size_t BytesRead = fread( *data, sizeof( BYTE ), size, f );
	fclose( f );
	return BytesRead;
}

#include <atomic>

UWorld *CurrentWorld = nullptr;

std::atomic<int> ProcessingState = 0;
void RenderThreadExport()
{
	ProcessAllMaterials();

	ProcessingState = 1;
}

class RenderThreadTaskInitializer
{
public:
	void QueCommand(  bool UseSync = false )
	{
		if( UseSync )
		{
			RenderThreadExport();
		}
		else
		{
			RenderThreadTaskInitializer* This = this;
			ENQUEUE_RENDER_COMMAND( Command_RenderThreadExport )(
				[This]( FRHICommandListImmediate& RHICmdList )
			{
				RenderThreadExport();
			} );
		}
	}
};
class UICallback
{
public:
	virtual void AddToolbarExtension()
	{

	}
};
RenderThreadTaskInitializer *RTTI = new RenderThreadTaskInitializer;

#endif

void DoExportScene( UWorld *World, bool ExportSingleFBX, const wchar_t* ExportFolder )
{
	wcscpy_s( UnityProjectFolder, ExportFolder );

	GUIDCounter = 0;
	
	#if WITH_EDITOR

	ProcessingState = 0;
	AllMaterials.clear();	
	AllTextures.clear();
	MeshList.clear();
	//MeshInstances.clear();
	
	std::wstring AssetsFolder = GetUnityAssetsFolder();
	std::wstring ShadersFolder = AssetsFolder + L"Shaders\\";
	std::wstring ScriptsFolder = AssetsFolder + L"Scripts\\";
	std::wstring ProjectSettingsFolder = AssetsFolder + L"..\\ProjectSettings\\";	

	if ( ClearMetas )
	{
		IFileManager& FileMan = IFileManager::Get();

		TArray<FString> DictionaryList;

		//std::wstring Wstr = GetUnityAssetsFolder();		
		const TCHAR *StartDirectory = AssetsFolder.c_str();// Wstr.c_str();
		FileMan.FindFilesRecursive( DictionaryList, StartDirectory, TEXT( "*.meta" ), true, false );

		for ( int i = 0; i < DictionaryList.Num(); i++ )
		{
			FString File = DictionaryList[ i ];
			bool Result = FPlatformFileManager::Get().GetPlatformFile().DeleteFile( *File );
			if (!Result )
			{
			}
		}

		bool RemoveLibrary = false;
		if( RemoveLibrary )
		{
			std::wstring LibraryDir = GetUnityAssetsFolder();
			LibraryDir += L"..\\Library";
			bool DeletedLibrary = FPlatformFileManager::Get().GetPlatformFile().DeleteDirectoryRecursively( LibraryDir.c_str() );
		}
		
		std::wstring MaterialsDir = GetUnityAssetsFolder();
		MaterialsDir += L"\\Materials";
		bool DeletedMaterials = FPlatformFileManager::Get().GetPlatformFile().DeleteDirectoryRecursively( MaterialsDir.c_str() );

		std::wstring MeshesDir = GetUnityAssetsFolder();
		MeshesDir += L"\\Meshes";
		bool DeletedMeshes = FPlatformFileManager::Get().GetPlatformFile().DeleteDirectoryRecursively( MeshesDir.c_str() );
	}
	

	CurrentWorld = World;

	VerifyOrCreateDirectory( UnityProjectFolder );
	VerifyOrCreateDirectory( AssetsFolder.c_str() );
	VerifyOrCreateDirectory( ShadersFolder.c_str() );
	VerifyOrCreateDirectory( ProjectSettingsFolder.c_str() );
	VerifyOrCreateDirectory( ScriptsFolder.c_str() );

	CopyToOutput( L"ProjectSettings.asset", L"ProjectSettings\\ProjectSettings.asset" );
	CopyToOutput( L"ProjectVersion.txt", L"ProjectSettings\\ProjectVersion.txt" );
	CopyToOutput( L"PostImportTool.cs", L"Assets/Scripts/PostImportTool.cs" );

	//GetUnityAssetsFolder()
	std::wstring SceneFile = GetUnityAssetsFolder();
	SceneFile += L"Scene.unity";

	UnityScene = new Scene();

	auto AllActors = CurrentWorld->GetProgressDenominator();

	FScopedSlowTask SlowTask( AllActors );
	SlowTask.MakeDialog();
	//SlowTask.EnterProgressFrame();

	for( TActorIterator<AActor> ActorItr( CurrentWorld ); ActorItr; ++ActorItr )
	{
		auto Current = ActorItr.GetProgressNumerator();		
		SlowTask.EnterProgressFrame();

		ProcessActor( *ActorItr, UnityScene );// , &Root, &DataStore, &Root );
	}

	for( int i = 0; i < AllMaterials.size(); i++ )
	{
		auto Material = AllMaterials[ i ];
		ExportMaterialTextures( AllMaterials[ i ] );
		GetMaterialShaderSource( AllMaterials[ i ], true );
		if ( Material->MaterialInterface )
			Material->BaseMaterial = Material->MaterialInterface->GetMaterial();
	}

	bool Sync = false;// true;
	RTTI->QueCommand( Sync );

	while( ProcessingState < 1 )
	{
		FPlatformProcess::Sleep( 0.01f );
	}

	for( int i = 0; i < AllTextures.size(); i++ )
	{
		auto Binding = AllTextures[ i ];

		size_t pos = Binding->UnityTex->File.find( "T_Pew_D.png" );
		if ( pos != -1 )
		{
			pos = pos;
		}
		if( !Binding->UnityTex )
		{
			//UE_LOG( LogEngine, Error, TEXT( "Texture %d has no UnityTex !!" ), i );
			continue;
		}

		std::string TexMetaFile = Binding->UnityTex->File;
		TexMetaFile += ".meta";

		int sRGB = Binding->UnrealTexture->SRGB;
		std::string TexMetaContents = GenerateTextureMeta( Binding->UnityTex->GUID, Binding->IsNormalMap, sRGB );
		if ( ExportTextures )
			::SaveFile( TexMetaFile.c_str(), (BYTE*)TexMetaContents.c_str(), TexMetaContents.length() );
	}

	for( int i = 0; i < AllMaterials.size(); i++ )
	{
		auto Mat = AllMaterials[ i ];

		std::wstring MatFile = GetUnityAssetsFolder();
		MatFile += L"\\Materials\\";

		VerifyOrCreateDirectory( MatFile.c_str() );

		MatFile += ToWideString( Mat->UnityMat->Name );
		MatFile += L".mat";

		std::string MatFileContents = Mat->UnityMat->GenerateMaterialFile();
		::SaveFile( ToANSIString( MatFile ).c_str(), (BYTE*)MatFileContents.c_str(), MatFileContents.length() );

		std::wstring MatMetaFile = MatFile;
		MatMetaFile += L".meta";
		
		Mat->UnityMat->GenerateGUID();
		std::string MaterialMetaContents = GenerateMaterialMeta( Mat->UnityMat->GUID );
		::SaveFile( ToANSIString( MatMetaFile ).c_str(), (BYTE*)MaterialMetaContents.c_str(), MaterialMetaContents.length() );		
	}

	//Export All OBJs
	for( int i = 0; i < MeshList.size(); i++ )
	{		
		MeshBinding *MB = MeshList[ i ];
		UnityMesh *M = MB->TheUnityMesh;

		std::wstring MeshFile = GetUnityAssetsFolder();
		MeshFile += L"Meshes\\";

		VerifyOrCreateDirectory( MeshFile.c_str() );

		MeshFile += ToWideString( M->Name );
		MeshFile += L".obj";
		
		ExportOBJ( ToANSIString( MeshFile ).c_str(), M);

		std::wstring MeshMetaFile = MeshFile;
		MeshMetaFile += L".meta";

		std::string MetaData = GenerateOBJMeta( M );// M->Name.c_str(), M->Sections.size(), & M->GUID );
		int MetaDataLen = MetaData.length();
		::SaveFile( ToANSIString( MeshMetaFile ).c_str(), (BYTE*)MetaData.c_str(), MetaDataLen );
	}

	bool WriteScene = !ExportSingleFBX;

	if( WriteScene )
	{
		FString ProxySceneFile = GetResourceDir();
		ProxySceneFile += TEXT( "Proxy.unity" );

		UnityScene->WriteScene( SceneFile.c_str(), *ProxySceneFile );
	}
	else
	{
		//Export FBX
		std::wstring FBXFile = GetUnityAssetsFolder();
		FBXFile += *( World->GetName() );
		FBXFile += L".fbx";
		GUnrealEd->ExportMap( World, FBXFile.c_str(), false );

		std::wstring FBXFileMeta = FBXFile;
		FBXFileMeta += L".meta";

		std::string MetaData = GenerateExportedFBXMeta();
		int MetaDataLen = MetaData.length();
		::SaveFile( ToANSIString( FBXFileMeta ).c_str(), (BYTE*)MetaData.c_str(), MetaDataLen );
	}

	#endif
}