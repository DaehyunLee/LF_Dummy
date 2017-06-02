// Fill out your copyright notice in the Description page of Project Settings.

#include "LF_Dummy.h"
#include "LFCameraRig.h"

#include "HighResScreenShot.h"


DEFINE_LOG_CATEGORY_STATIC(RigSimulator, Log, All);

// Sets default values
ALFCameraRig::ALFCameraRig()
	:NumCameraXbyX(32, 32)
	, CameraGap_mm (10)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	OutputPath = "D:/LF_output";
}

// Called when the game starts or when spawned
void ALFCameraRig::BeginPlay()
{
	Super::BeginPlay();
	
	resolution = 256;

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

			info.CameraPosition.X = ix;
			info.CameraPosition.Y = iy;

			info.Pos.X = GetActorLocation().X ;
			info.Pos.Y = GetActorLocation().Y + ix*CameraGap_mm;
			info.Pos.Z = GetActorLocation().Z - iy*CameraGap_mm;

			info.Rot = GetActorRotation();
		}
	}

	//spawn helper lines 
	SpawnHelperLines();

	//Spawn actual camera.
	for (const auto & info : CameraInformation)
	{
		CaptureCamera camera;
		camera.info = info;
		camera.capture2d = SpawnCamera(info);

		Cameras.Add(camera);
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
				lTextureRender->InitAutoFormat(resolution, resolution);
				lTextureRender->UpdateResource();

				captureComp->Deactivate();
				captureComp->FOVAngle = 120.f;
				captureComp->TextureTarget = lTextureRender;
				captureComp->bCaptureEveryFrame = false;
				captureComp->bAutoActivate = false;
				captureComp->DetailMode = EDetailMode::DM_High;
				captureComp->PostProcessSettings.AutoExposureMinBrightness = 1.0f;
				captureComp->PostProcessSettings.AutoExposureMaxBrightness = 1.0f;
				captureComp->CaptureSource = SCS_FinalColorLDR;

				FEngineShowFlagsSetting *setting;
				if (captureComp->GetSettingForShowFlag("Fog", &setting))
				{
					(*setting).Enabled = false;
				}

				captureComp->UpdateContent();
				captureComp->PostEditChange();
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
	UE_LOG(RigSimulator, Display, TEXT("trigger all camera."));
	for (const auto& entry : Cameras)
	{
		USceneCaptureComponent2D*  capture = entry.capture2d->GetCaptureComponent2D();
		capture->UpdateContent();
	}

	SaveAllCamera_Single();
}

void ALFCameraRig::SaveAllCamera_Single()
{
	TArray<FColor> OutBMP_SingleImage;
	FIntPoint singeImageSize = NumCameraXbyX * resolution;
	OutBMP_SingleImage.SetNum(singeImageSize.X * singeImageSize.Y);

	for (const auto& entry : Cameras)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		UTextureRenderTarget2D* t = entry.capture2d->GetCaptureComponent2D()->TextureTarget;
		check(t);

		FTextureRenderTargetResource* RTResource = t->GameThread_GetRenderTargetResource();

		FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
		ReadPixelFlags.SetLinearToGamma(true);

		TArray<FColor> OutBMP;
		RTResource->ReadPixels(OutBMP, ReadPixelFlags);

		CopyColorBuffer(OutBMP, resolution, entry.info.CameraPosition, OutBMP_SingleImage);
	}

	FString filename = TEXT("LF_SingleOutput.png");
	FString fullPath = OutputPath;
	fullPath /= filename;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	int postfix = 1;
	while (PlatformFile.FileExists(*fullPath))
	{
		FString filename = TEXT("LF_SingleOutput_") + FString::FormatAsNumber(postfix) + TEXT(".png");
		fullPath = OutputPath / filename;
		postfix++;
	}

	FString ResultPath;
	FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();
	HighResScreenshotConfig.SaveImage(fullPath, OutBMP_SingleImage, singeImageSize, &ResultPath);
	UE_LOG(RigSimulator, Log, TEXT("file written. (%s)"), *ResultPath);
}

void ALFCameraRig::CopyColorBuffer(const TArray<FColor>& input, int resolution, FIntPoint position, TArray<FColor>& output)
{
	FIntPoint fullResolution(NumCameraXbyX.X * resolution, NumCameraXbyX.Y * resolution);
	FIntPoint pointInOutput(position.X * resolution, position.Y * resolution);

	for (int ix = 0; ix < resolution; ix++)
		for (int iy = 0; iy < resolution; iy++)
		{
			int inputIndex = ix + iy*resolution;
			int outputIndex = (ix + pointInOutput.X) + ((iy + pointInOutput.Y)* fullResolution.X);

			output[outputIndex] = input[inputIndex];
			output[outputIndex].A = 255;
		}
}


void ALFCameraRig::SaveAllCamera_Separate()
{
	for (const auto& entry : Cameras)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		ASceneCapture2D *capture = entry.capture2d;

		FString filename = FString::Printf(TEXT("rigSimCalib%s.png"), *capture->GetActorLabel());

		FString fullPath = OutputPath;
		fullPath /= filename;
		if (UTextureRenderTarget2D* t = capture->GetCaptureComponent2D()->TextureTarget)
		{
			SaveRenderTargetToDisk(t, fullPath);
		}

		UE_LOG(RigSimulator, Log, TEXT("writing png for (%s)"), *capture->GetName());
	}
}


void ALFCameraRig::SaveRenderTargetToDisk(UTextureRenderTarget2D* InRenderTarget, FString filename)
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
	HighResScreenshotConfig.SaveImage(filename, OutBMP, DestSize, &ResultPath);
	UE_LOG(RigSimulator, Log, TEXT("file written. (%s)"), *ResultPath);
}
