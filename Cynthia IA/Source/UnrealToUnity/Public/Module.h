// Copyright 2018 GiM s.r.o. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FUnrealToUnityModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void BindEditorCommands();
	void AddCustomMenu( FToolBarBuilder& ToolbarBuilder );
	TSharedRef<SWidget> CreateMenuContent();
	void ExportOBJs();
	void ExportAsSingleFBX();
	void RemoveRVTs();

	TSharedPtr<FUICommandList> EditorCommands;
};


class FUnrealToUnity_Commands : public TCommands<FUnrealToUnity_Commands>
{
public:
	FUnrealToUnity_Commands()
		: TCommands<FUnrealToUnity_Commands>( "UnrealToUnityEditor", NSLOCTEXT( "Contexts", "UnrealToUnityEditor", "UnrealToUnityEditor" ), NAME_None, FEditorStyle::GetStyleSetName() )
	{
	}

	TSharedPtr<FUICommandInfo> ExportOBJs;
	TSharedPtr<FUICommandInfo> ExportAsSingleFBX;
	TSharedPtr<FUICommandInfo> RemoveRVTs;
	
	virtual void RegisterCommands() override;
};