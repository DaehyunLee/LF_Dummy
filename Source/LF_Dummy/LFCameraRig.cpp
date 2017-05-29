// Fill out your copyright notice in the Description page of Project Settings.

#include "LF_Dummy.h"
#include "LFCameraRig.h"


// Sets default values
ALFCameraRig::ALFCameraRig()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALFCameraRig::BeginPlay()
{
	Super::BeginPlay();
	
	NumCameraXbyX = FVector2D(9, 9);
	CameraGap_mm = 10;

	SpawnCameras();
}

// Called every frame
void ALFCameraRig::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	DrawHelperLines();
}

void ALFCameraRig::SpawnCameras()
{
	//create camera informations first
	CameraInformation.InsertUninitialized(0, NumCameraXbyX.X * NumCameraXbyX.Y);
	for (int ix = 0; ix < NumCameraXbyX.X; ix++)
	{
		for (int iy = 0; iy < NumCameraXbyX.Y; iy++)
		{
			CameraInfo& info = CameraInformation[iy*NumCameraXbyX.X + ix];

			info.Pos.X = GetActorLocation().X ;
			info.Pos.Y = GetActorLocation().Y + ix*CameraGap_mm;
			info.Pos.Z = GetActorLocation().Z + iy*CameraGap_mm;
		}
	}
}

void ALFCameraRig::DrawHelperLines()
{
	float scale = 0.5f;

	for (auto& info : CameraInformation)
	{
		DrawDebugCamera(GetWorld(), info.Pos, this->GetActorRotation(), 60.f, scale, FColor::Red);
	}
}
 