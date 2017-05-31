// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LFCameraRig.generated.h"


struct CameraInfo
{
	FIntPoint CameraPosition;

	FVector Pos;
	FRotator Rot;
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
	FIntPoint NumCameraXbyX;

	UPROPERTY(EditAnywhere)
	float CameraGap_mm;

	UPROPERTY(EditAnywhere)
	ASceneCapture2D* ReferenceForClone;

	UPROPERTY(EditAnywhere)
	FString OutputPath;

	UFUNCTION(BlueprintCallable, Category = "ScreenCapture")
	void TriggerAllCamera();

protected:
	struct CaptureCamera
	{
		CameraInfo info;
		class ASceneCapture2D* capture2d;
	};


	void SpawnCameras();
	ASceneCapture2D* SpawnCamera(const CameraInfo& info) const;
	void SpawnHelperLines();

	//capture related 
	void SaveAllCamera_Separate();
	void SaveAllCamera_Single();

	void CopyColorBuffer(const TArray<FColor>& input, int resolution, FIntPoint position, TArray<FColor>& output);

	void ALFCameraRig::SaveRenderTargetToDisk(UTextureRenderTarget2D* InRenderTarget, FString filename);

	TArray<CameraInfo> CameraInformation;
	TArray<CaptureCamera> Cameras;

private:
	int resolution;
};
