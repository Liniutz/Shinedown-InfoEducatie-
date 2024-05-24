// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShinedownPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class UNTITLEDPROJECT_API AShinedownPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);
private:
	class AUntitledProjectCharacter* Character;
	class AShinedownPlayerController* Controller;

	int32 Defeats;
};
