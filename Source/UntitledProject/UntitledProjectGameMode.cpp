// Copyright Epic Games, Inc. All Rights Reserved.

#include "UntitledProjectGameMode.h"
#include "UntitledProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUntitledProjectGameMode::AUntitledProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
