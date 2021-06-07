
#include "Module.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#include "LevelEditor.h"
#include "ExportActor.h"

#include "AssetRegistryModule.h"
#include <functional>

#include "Editor/MaterialEditor/Public/MaterialEditingLibrary.h"
#include "Engine/Classes/Materials/MaterialExpressionTextureCoordinate.h"
#include "Engine/Classes/Materials/MaterialExpressionRuntimeVirtualTextureSample.h"
#include "Engine/Classes/Materials/MaterialExpressionRuntimeVirtualTextureSampleParameter.h"
#include "Engine/Classes/Materials/MaterialExpressionConstant4Vector.h"

#define LOCTEXT_NAMESPACE "FUnrealToUnityModule"

void FUnrealToUnityModule::StartupModule()
{
	//return;
	FUnrealToUnity_Commands::Register();
	BindEditorCommands();
	
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>( "LevelEditor" );
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable( new FExtender );
	ToolbarExtender->AddToolBarExtension(
		"Game",
		EExtensionHook::After,
		NULL,
		FToolBarExtensionDelegate::CreateRaw( this, &FUnrealToUnityModule::AddCustomMenu )
	);

	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender( ToolbarExtender );
}

#define MapActionHelper(x) EditorCommands->MapAction( Commands.x, FExecuteAction::CreateRaw(this, &FUnrealToUnityModule::x), FCanExecuteAction(), FIsActionChecked())

void FUnrealToUnityModule::BindEditorCommands()
{
	if( !EditorCommands.IsValid() )
	{
		EditorCommands = MakeShareable( new FUICommandList() );
	}

	const FUnrealToUnity_Commands& Commands = FUnrealToUnity_Commands::Get();

	MapActionHelper( ExportOBJs );
	MapActionHelper( ExportAsSingleFBX );
	MapActionHelper( RemoveRVTs );	
}
void FUnrealToUnityModule::ExportOBJs()
{
	std::wstring StrOutputFolder = TEXT( "C:\\UnrealToUnity\\" );
	AExportActor::DoExport( StrOutputFolder, false );
}
void FUnrealToUnityModule::ExportAsSingleFBX()
{
	std::wstring StrOutputFolder = TEXT( "C:\\UnrealToUnity\\" );
	AExportActor::DoExport( StrOutputFolder, true );
}
void FUnrealToUnityModule::AddCustomMenu( FToolBarBuilder& ToolbarBuilder )
{
	ToolbarBuilder.BeginSection( "UnrealToUnity" );
	{
		ToolbarBuilder.AddComboButton(
			FUIAction()
			, FOnGetContent::CreateRaw( this, &FUnrealToUnityModule::CreateMenuContent )
			, LOCTEXT( "UnrealToUnityLabel", "UnrealToUnity" )
			, LOCTEXT( "UnrealToUnityTooltip", "UnrealToUnity" )
			, FSlateIcon( FEditorStyle::GetStyleSetName(), "LevelEditor.GameSettings" )
		);
	}
	ToolbarBuilder.EndSection();
}
TSharedRef<SWidget> FUnrealToUnityModule::CreateMenuContent()
{
	FMenuBuilder MenuBuilder( true, EditorCommands );
	
	MenuBuilder.AddMenuEntry( FUnrealToUnity_Commands::Get().ExportOBJs );
	MenuBuilder.AddMenuEntry( FUnrealToUnity_Commands::Get().ExportAsSingleFBX );
	MenuBuilder.AddMenuEntry( FUnrealToUnity_Commands::Get().RemoveRVTs );

	return MenuBuilder.MakeWidget();
}

void FUnrealToUnity_Commands::RegisterCommands()
{
	UI_COMMAND( ExportOBJs, "Export as OBJs", "Export current scene to a unity project", EUserInterfaceActionType::Button, FInputChord() );
	UI_COMMAND( ExportAsSingleFBX, "ExportAsSingleFBX", "Export current scene to a unity project", EUserInterfaceActionType::Button, FInputChord() );
	UI_COMMAND( RemoveRVTs, "RemoveRVTs", "Removes Runtime Virtual Textures from all materials", EUserInterfaceActionType::Button, FInputChord() );
}

void FUnrealToUnityModule::ShutdownModule()
{
}

void GetOutputExpression( FExpressionInput* MaterialInput, UMaterialExpression* Source, int OutputIndex, TArray< FExpressionInput*>& Inputs )
{
	if( MaterialInput->Expression == Source && MaterialInput->OutputIndex == OutputIndex )
		Inputs.Add( MaterialInput );
}
void GetOutputExpression( UMaterial* Material, UMaterialExpression* Source, int OutputIndex, TArray< FExpressionInput*>& Inputs )
{
	TArray<UMaterialExpression*> AllExpressions;
	Material->GetAllReferencedExpressions( AllExpressions, nullptr );

	for( int i = 0; i < AllExpressions.Num(); i++ )
	{
		UMaterialExpression* Exp = AllExpressions[ i ];

		TArray<FExpressionInput*> ExpInputs = Exp->GetInputs();
		for( int u = 0; u < ExpInputs.Num(); u++ )
		{
			UMaterialExpression* A = ExpInputs[ u ]->Expression;
			if( A == Source && ExpInputs[ u ]->OutputIndex == OutputIndex )
			{
				Inputs.Add( ExpInputs[ u ] );
			}
		}
	}

	GetOutputExpression( &Material->BaseColor, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->Metallic, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->Specular, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->Roughness, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->Anisotropy, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->Normal, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->Tangent, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->EmissiveColor, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->Opacity, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->OpacityMask, Source, OutputIndex, Inputs );

	GetOutputExpression( &Material->WorldPositionOffset, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->WorldDisplacement, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->TessellationMultiplier, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->SubsurfaceColor, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->ClearCoat, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->ClearCoatRoughness, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->AmbientOcclusion, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->Refraction, Source, OutputIndex, Inputs );

	for( int i = 0; i < 8; i++ )
		GetOutputExpression( &Material->CustomizedUVs[ i ], Source, OutputIndex, Inputs );

	GetOutputExpression( &Material->PixelDepthOffset, Source, OutputIndex, Inputs );
	GetOutputExpression( &Material->ShadingModelFromMaterialExpression, Source, OutputIndex, Inputs );
}
UMaterialExpression* CreateRVTReplacementExpression( FExpressionOutput& OE, UMaterial* Material, FVector2D& Location )
{
	UMaterialExpression* NewExpression = nullptr;

	if( OE.OutputName.ToString().Compare( "BaseColor" ) == 0 ||
		OE.OutputName.ToString().Compare( "Normal" ) == 0 )
	{
		//1, 1, 1, 0
		NewExpression = UMaterialEditingLibrary::CreateMaterialExpressionEx( Material, nullptr, UMaterialExpressionConstant4Vector::StaticClass(), nullptr, Location.X, Location.Y );
		UMaterialExpressionConstant4Vector* NewExpC = Cast < UMaterialExpressionConstant4Vector>( NewExpression );

		if( OE.OutputName.ToString().Compare( "BaseColor" ) == 0 )
		{
			NewExpC->Constant.R = NewExpC->Constant.G = NewExpC->Constant.B = 1;
			NewExpC->Constant.A = 0;
		}
		else
		{
			NewExpC->Constant.R = 0;
			NewExpC->Constant.G = 0;
			NewExpC->Constant.B = 1;
			NewExpC->Constant.A = 0;
		}

		Location.Y += 140;
	}
	else if( OE.OutputName.ToString().Compare( "Specular" ) == 0 ||
			 OE.OutputName.ToString().Compare( "Roughness" ) == 0 ||
			 OE.OutputName.ToString().Compare( "WorldHeight" ) == 0 ||
			 OE.OutputName.ToString().Compare( "Mask" ) == 0 )
	{
		NewExpression = UMaterialEditingLibrary::CreateMaterialExpressionEx( Material, nullptr, UMaterialExpressionConstant::StaticClass(), nullptr, Location.X, Location.Y );
		UMaterialExpressionConstant* NewExpC = Cast < UMaterialExpressionConstant>( NewExpression );

		if( OE.OutputName.ToString().Compare( "Roughness" ) == 0 )
			NewExpC->R = 1;
		else
			NewExpC->R = 0;

		Location.Y += 60;
	}

	return NewExpression;
}
void IterateOverAllAssetsOfType( FName TypeName, std::function<void( FAssetData& )> lambda )
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>( "AssetRegistry" );

	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAssetsByClass( TypeName, Assets );

	int current = 0;
	int Offset = 0;
	for( FAssetData& Asset : Assets )
	{
		if( Offset <= current )
		{
			//Always ignore stuff from /Developers or engine data
			if( Asset.PackageName.ToString().Contains( "/Game/Developers/" ) ||
				Asset.PackageName.ToString().StartsWith( "/Engine" ) )
				continue;

			lambda( Asset );
		}
		current++;
	}
}
void FUnrealToUnityModule::RemoveRVTs()
{
	TArray<FString> Messages;
	auto lambda = [&]( FAssetData& Asset )
	{
		UMaterial* Material = Cast<UMaterial>( Asset.GetAsset() );
		if( Material )
		{
			bool Modified = false;
			TArray<UMaterialExpression*> OutExpressions;
			Material->GetAllReferencedExpressions( OutExpressions, nullptr );
			for( int i = 0; i < OutExpressions.Num(); i++ )
			{
				UMaterialExpression* Exp = OutExpressions[ i ];
				UMaterialExpressionRuntimeVirtualTextureSample* RVTExp = Cast< UMaterialExpressionRuntimeVirtualTextureSample>( Exp );
				UMaterialExpressionRuntimeVirtualTextureSampleParameter* RVTExp2 = Cast< UMaterialExpressionRuntimeVirtualTextureSampleParameter>( Exp );
				if( RVTExp || RVTExp2 )
				{
					TArray<FExpressionOutput>& Outputs = Exp->GetOutputs();

					FVector2D Location( Exp->MaterialExpressionEditorX + 250, Exp->MaterialExpressionEditorY );

					for( int o = 0; o < Outputs.Num(); o++ )
					{
						FExpressionOutput& OE = Outputs[ o ];
						TArray< FExpressionInput*> Inputs;
						GetOutputExpression( Material, Exp, o, Inputs );
						if( Inputs.Num() > 0 )
						{
							auto NewExp = CreateRVTReplacementExpression( OE, Material, Location );

							for( int j = 0; j < Inputs.Num(); j++ )
							{
								auto ExpInput = Inputs[ j ];
								ExpInput->Expression = NewExp;
							}
							Modified = true;
						}
					}
				}
			}

			if( Modified )
			{
				Material->PostEditChange();
				Material->MarkPackageDirty();
			}
		}
	};

	IterateOverAllAssetsOfType( FName( "Material" ), lambda );
	//IterateOverSelection( lambda );
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE( FUnrealToUnityModule, UnrealToUnity )