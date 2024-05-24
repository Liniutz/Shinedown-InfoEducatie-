// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "WeaponTypes.h"
#include "CombatState.h"

#include "CombatComponent.generated.h"


class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNTITLEDPROJECT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AUntitledProjectCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	int32 AmountToReload();

	void SwapWeapons();
	void PerformMelee();

	void EquipWeapon(AWeapon* WeaponToEquip);
	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToBackpack(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);
protected:
	virtual void BeginPlay() override;

	void FireButtonPressed(bool bPressed);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

	void HandleReload();
private:
	UPROPERTY()
	class AUntitledProjectCharacter* Character;

	UPROPERTY()
	class AShinedownPlayerController* PlayerController;

	UPROPERTY()
	class AShinedownHUD* HUD;

	UPROPERTY()
	class AWeapon* EquippedWeapon;

	UPROPERTY()
	AWeapon* SecondaryWeapon;

	bool bFireButtonPressed;

	FVector HitTarget;
	/*
	* HUD and Crosshairs
	*/

	float CrosshairVelocityFactor;
	float CrosshairShootingFactor;
	float CrosshaiInAirFactor;

	/*
	* Automatic fire
	*/
	FTimerHandle FireTimer;

	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();

	void Fire();
	void AIFire();

	bool CanFire();

	UPROPERTY()
	int32 CarriedAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRLAmmo = 4;
	
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 60;
	
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 4;


	void InitializeCarriedAmmo();

	UPROPERTY()
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;

public:	
	bool ShouldSwapWeapons();
		
};
