// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "DemoCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class DEMO_API ADemoCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	ADemoCameraManager();
	
	// Camera Сrouch
	UPROPERTY(EditDefaultsOnly) float CrouchBlendDuration = .3f;
	float CrouchBlendTime;
	
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
};
