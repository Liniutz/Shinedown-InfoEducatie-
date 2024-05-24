// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "UPPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class UNTITLEDPROJECT_API AUPPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	public:
	AUPPlayerCameraManager();

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

private:
	UPROPERTY(EditDefaultsOnly)
	float CrouchBlendDuration = .2f;

	float CrouchBlendTime;

	FRotator OriginalRotation;
};
