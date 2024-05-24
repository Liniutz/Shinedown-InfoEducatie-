// Fill out your copyright notice in the Description page of Project Settings.


#include "ShinedownPlayerController.h"
#include "ShinedownHUD.h"
#include "CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
//#include "UntitledProjectCharacter.h"


void AShinedownPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	ShinedownHUD = ShinedownHUD == nullptr ? Cast<AShinedownHUD>(GetHUD()) : ShinedownHUD;
	bool bHUDValid = ShinedownHUD && ShinedownHUD->CharacterOverlay &&
		ShinedownHUD->CharacterOverlay->HealthBar && ShinedownHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		ShinedownHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Health));
		ShinedownHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void AShinedownPlayerController::SetHUDScore(float Score)
{
	ShinedownHUD = ShinedownHUD == nullptr ? Cast<AShinedownHUD>(GetHUD()) : ShinedownHUD;
	bool bHUDValid = ShinedownHUD &&
		ShinedownHUD->CharacterOverlay &&
		ShinedownHUD->CharacterOverlay->ScoreText;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		ShinedownHUD->CharacterOverlay->ScoreText->SetText(FText::FromString(ScoreText));
	}
}

void AShinedownPlayerController::SetHUDDefeats(int32 Defeats)
{
	ShinedownHUD = ShinedownHUD == nullptr ? Cast<AShinedownHUD>(GetHUD()) : ShinedownHUD;
	bool bHUDValid = ShinedownHUD &&
		ShinedownHUD->CharacterOverlay &&
		ShinedownHUD->CharacterOverlay->DefeatsText;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		ShinedownHUD->CharacterOverlay->DefeatsText->SetText(FText::FromString(DefeatsText));
	}
}

void AShinedownPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	ShinedownHUD = ShinedownHUD == nullptr ? Cast<AShinedownHUD>(GetHUD()) : ShinedownHUD;
	bool bHUDValid = ShinedownHUD && ShinedownHUD->CharacterOverlay &&
		ShinedownHUD->CharacterOverlay->ShieldBar && ShinedownHUD->CharacterOverlay->ShieldText;
	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		ShinedownHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Shield));
		ShinedownHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
}

void AShinedownPlayerController::SetHUDAmmo(int32 Ammo)
{
	ShinedownHUD = ShinedownHUD == nullptr ? Cast<AShinedownHUD>(GetHUD()) : ShinedownHUD;
	bool bHUDValid = ShinedownHUD && ShinedownHUD->CharacterOverlay &&
		ShinedownHUD->CharacterOverlay->AmmoText;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ShinedownHUD->CharacterOverlay->AmmoText->SetText(FText::FromString(AmmoText));
	}
}

void AShinedownPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	ShinedownHUD = ShinedownHUD == nullptr ? Cast<AShinedownHUD>(GetHUD()) : ShinedownHUD;
	bool bHUDValid = ShinedownHUD && ShinedownHUD->CharacterOverlay &&
		ShinedownHUD->CharacterOverlay->CarriedAmmoText;
	if (bHUDValid)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), Ammo);
		ShinedownHUD->CharacterOverlay->CarriedAmmoText->SetText(FText::FromString(CarriedAmmoText));
	}
}

void AShinedownPlayerController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
{
	Super::GameHasEnded(EndGameFocus, bIsWinner);
	bWinLoss = bIsWinner;
	StartShowWidgetTimer();
}

void AShinedownPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ShinedownHUD = ShinedownHUD == nullptr ? Cast<AShinedownHUD>(GetHUD()) : ShinedownHUD;

}

void AShinedownPlayerController::PollInit()
{
}

void AShinedownPlayerController::StartShowWidgetTimer()
{
	GetWorldTimerManager().SetTimer(
		ShowWidgetTimer,
		this,
		&ThisClass::ShowWidgetTimerfinished,
		2.5f
	);
}

void AShinedownPlayerController::ShowWidgetTimerfinished()
{
	StopMovement();
	 
	ShinedownHUD->CharacterOverlay->SetVisibility(ESlateVisibility::Hidden);
	bShowMouseCursor = true;
	Pause();
	ShinedownHUD->SetHidden(true);
	if (!bWinLoss) {
		UUserWidget* DeathScreen = CreateWidget(this, DeathWidget);
		if (DeathScreen)
		{
			DeathScreen->AddToViewport();
		}
	}
	else
	{
		UUserWidget* WonScreen = CreateWidget(this, WonWidget);
		if (WonScreen)
		{
			WonScreen->AddToViewport();
		}
	}
}
