// Fill out your copyright notice in the Description page of Project Settings.

#include "LF_Dummy.h"
#include "LFCameraRig.h"

#include "HighResScreenShot.h"


DEFINE_LOG_CATEGORY_STATIC(RigSimulator, Log, All);

// Sets default values
ALFCameraRig::ALFCameraRig()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	OutputPath = "D:/LF_output";
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

			info.Rot = GetActorRotation();
		}
	}

	//spawn helper lines 
	SpawnHelperLines();

	//Spawn actual camera.
	for (const auto & info : CameraInformation)
	{
		Cameras.Add(SpawnCamera(info));
	}

}

ASceneCapture2D* ALFCameraRig::SpawnCamera(const CameraInfo& info) const
{
	//calculate position from rotation
	// rot seems to represent pitch yaw roll
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Template = ReferenceForClone;
	if (ASceneCapture2D* myCapture = GetWorld()->SpawnActor<ASceneCapture2D>(info.Pos, info.Rot, SpawnInfo))
	{
		if (USceneCaptureComponent2D* captureComp = myCapture->GetCaptureComponent2D())
		{
			if (class UTextureRenderTarget2D* lTextureRender = NewObject<UTextureRenderTarget2D>())
			{
				//lTexture->SizeX = 512.f;
				//lTexture->SizeY = 512.f;
				lTextureRender->InitAutoFormat(1296.f, 1032.f);
				lTextureRender->UpdateResource();

				captureComp->Deactivate();
				captureComp->FOVAngle = 60.f;
				captureComp->TextureTarget = lTextureRender;
				captureComp->bCaptureEveryFrame = false;
				captureComp->bAutoActivate = false;
				captureComp->DetailMode = EDetailMode::DM_High;
				captureComp->PostProcessSettings.AutoExposureMinBrightness = 1.0f;
				captureComp->PostProcessSettings.AutoExposureMaxBrightness = 1.0f;

				FEngineShowFlagsSetting *setting;
				if (captureComp->GetSettingForShowFlag("Fog", &setting))
				{
					(*setting).Enabled = false;
				}

				captureComp->UpdateContent();
				captureComp->PostEditChange();
				//lSCC->Activate();
				return myCapture;
			}
		}
	}
	return nullptr;
}


void ALFCameraRig::SpawnHelperLines()
{
	float scale = 0.5f;

	for (auto& info : CameraInformation)
	{
		DrawDebugCamera(GetWorld(), info.Pos, info.Rot, 60.f, scale, FColor::Red, true);
	}
}
 
void ALFCameraRig::TriggerAllCamera()
{
	UE_LOG(RigSimulator, Display, TEXT("taking screenshot."));

	for (const auto& entry : Cameras)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		ASceneCapture2D *capture = entry;

		FString filename = FString::Printf(TEXT("rigSimCalib%s.png"), *capture->GetActorLabel());

		FString fullPath = OutputPath;
		fullPath /= filename;
		if (UTextureRenderTarget2D* t = entry->GetCaptureComponent2D()->TextureTarget)
		{
			SaveRenderTargetToDisk(t, fullPath);
		}

//		capture->TakeSnapshot(*filename);
		UE_LOG(RigSimulator, Log, TEXT("writing png for (%s)"), *capture->GetName());

		//WriteToJson(*capture);
	}
}

void ALFCameraRig::SaveRenderTargetToDisk(UTextureRenderTarget2D* InRenderTarget, FString Filename)
{
	FTextureRenderTargetResource* RTResource = InRenderTarget->GameThread_GetRenderTargetResource();

	FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
	ReadPixelFlags.SetLinearToGamma(true);

	TArray<FColor> OutBMP;
	RTResource->ReadPixels(OutBMP, ReadPixelFlags);

	for (FColor& color : OutBMP)
	{
		color.A = 255;
	}

	FIntRect SourceRect;
	FIntPoint DestSize(InRenderTarget->GetSurfaceWidth(), InRenderTarget->GetSurfaceHeight());

	FString ResultPath;
	FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();
	HighResScreenshotConfig.SaveImage(Filename, OutBMP, DestSize, &ResultPath);
	UE_LOG(RigSimulator, Log, TEXT("file written. (%s)"), *ResultPath);
}
