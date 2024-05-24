// Fill out your copyright notice in the Description page of Project Settings.


#include "ShinedownPlayerState.h"
#include "UntitledProjectCharacter.h"
#include "ShinedownPlayerController.h"
#include "Net/UnrealNetwork.h"

void AShinedownPlayerState::AddToScore(float ScoreAmount)
{
	Score += ScoreAmount;
	Character = Character == nullptr ? Cast<AUntitledProjectCharacter>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		Character->Score = Score;
		Controller = Controller == nullptr ? Cast<AShinedownPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}
}

void AShinedownPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AUntitledProjectCharacter>(GetPawn()) : Character;
	if (Character && Character->Controller)
	{
		Controller = Controller == nullptr ? Cast<AShinedownPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}
