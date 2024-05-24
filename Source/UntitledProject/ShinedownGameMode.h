// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShinedownGameMode.generated.h"

/**
 * 
 */
UCLASS()
class UNTITLEDPROJECT_API AShinedownGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	void EndMatch() override;
};
