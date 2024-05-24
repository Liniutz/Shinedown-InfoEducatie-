// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShinedownPlayerController.generated.h"


UCLASS()
class UNTITLEDPROJECT_API AShinedownPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	virtual void GameHasEnded(class AActor* EndGameFocus = nullptr, bool bIsWinner = false) override;
protected:
	virtual void BeginPlay() override;
	void PollInit();
private:
	UPROPERTY()
	class AShinedownHUD* ShinedownHUD;

	float HUDHealth;
	bool bInitializeHealth = false;
	float HUDMaxHealth;
	float HUDShield;
	bool bInitializeShield = false;
	float HUDMaxShield;
	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;
	
	bool bShowWidgets;
	FTimerHandle ShowWidgetTimer;

	void StartShowWidgetTimer();
	void ShowWidgetTimerfinished();
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> DeathWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> WonWidget;


	bool bWinLoss = false;
public:
	FORCEINLINE class AShinedownHUD* GetHud() const { return ShinedownHUD; }
};
