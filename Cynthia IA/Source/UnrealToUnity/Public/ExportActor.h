// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <string>

#include "ExportActor.generated.h"

UCLASS()
class AExportActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExportActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY( EditAnywhere )
	bool ExportSingleFBX = false;

	UPROPERTY( EditAnywhere )
	FString OutputFolder = TEXT( "C:\\UnrealToUnity\\" );

	//UFUNCTION( BlueprintCallable )
	

	UFUNCTION( BlueprintCallable )
	void EventExport();

	static void DoExport( std::wstring StrOutputFolder, bool ExportSingleFBX );
	static void RotateActorsForExport( bool Forward );
	static void DetachAllActors();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
