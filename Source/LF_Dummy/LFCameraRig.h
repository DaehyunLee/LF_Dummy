// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LFCameraRig.generated.h"


struct CameraInfo
{
	FVector Pos;
};

UCLASS()
class LF_DUMMY_API ALFCameraRig : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALFCameraRig();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(EditAnywhere)
	FVector2D NumCameraXbyX;

	UPROPERTY(EditAnywhere)
	float CameraGap_mm;

protected:
	void SpawnCameras();
	void DrawHelperLines();


	TArray<CameraInfo> CameraInformation;

};
