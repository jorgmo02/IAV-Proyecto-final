// Fill out your copyright notice in the Description page of Project Settings.

#include "ExportActor.h"
#include "ExportToUnity.h"
#include "EngineUtils.h"

// Sets default values
AExportActor::AExportActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AExportActor::BeginPlay()
{
	Super::BeginPlay();
	
	EventExport();
}

// Called every frame
void AExportActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void AExportActor::EventExport()
{
	DoExport( *OutputFolder, ExportSingleFBX );
}
bool DoRotateOnExport = true;
void AExportActor::DoExport( std::wstring StrOutputFolder, bool ExportSingleFBX )
{
	DetachAllActors();

	if ( !ExportSingleFBX && DoRotateOnExport )
		RotateActorsForExport( true );

	UWorld* world = GEditor->GetEditorWorldContext( false ).World();
	DoExportScene( world, ExportSingleFBX, StrOutputFolder.c_str() );

	if ( !ExportSingleFBX && DoRotateOnExport )
		RotateActorsForExport( false );
}
void AExportActor::DetachAllActors()
{
	UWorld* world = GEditor->GetEditorWorldContext( false ).World();

	for( TActorIterator<AActor> ActorItr( world ); ActorItr; ++ActorItr )
	{
		AActor* Actor = *ActorItr;
		checkSlow( Actor );

		USceneComponent* RootComp = Actor->GetRootComponent();
		if( RootComp != nullptr && RootComp->GetAttachParent() != nullptr )
		{
			AActor* OldParentActor = RootComp->GetAttachParent()->GetOwner();
			OldParentActor->Modify();
			RootComp->DetachFromComponent( FDetachmentTransformRules::KeepWorldTransform );
			Actor->SetFolderPath_Recursively( OldParentActor->GetFolderPath() );
		}

		//Detach components
		//TArray<USceneComponent*> comps;
		//Actor->GetComponents( comps );
		//for( int i = 0; i < comps.Num(); i++ )
		//{
		//	USceneComponent* Comp = comps[ i ];
		//	Comp->DetachFromComponent( FDetachmentTransformRules::KeepWorldTransform );
		//}
	}
}
void AExportActor::RotateActorsForExport( bool Forward )
{
	UWorld* world = GEditor->GetEditorWorldContext( false ).World();

	GEditor->SelectNone( false, true, false );
	
	for( TActorIterator<AActor> ActorItr( world ); ActorItr; ++ActorItr )
	{
		AActor* actor = *ActorItr;

		const AActor* Parent = actor->GetParentActor();
		//Don't modify children
		if( Parent )
			continue;

		//GEditor->SelectActor( actor, /*bSelected=*/ false, /*bNotify=*/ false );
		FVector InDeltaDrag(0,0,0);
		float Value = 90.0f;
		if( !Forward )
			Value *= -1;
		FRotator InDeltaRot(0,0,Value);
		FVector InDeltaScale( 0, 0, 0 );
		FVector ModifiedDeltaScale = InDeltaScale;
		GEditor->ApplyDeltaToActor(
			actor,
			true,
			&InDeltaDrag,
			&InDeltaRot,
			&ModifiedDeltaScale,
			false,//IsAltPressed(),
			false,//IsShiftPressed(),
			false//IsCtrlPressed() 
			);
	}
}

