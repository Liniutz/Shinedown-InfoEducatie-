// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CombatState.h"
#include "UntitledProjectCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

UCLASS(config=Game)
class AUntitledProjectCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MeleeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwapWeaponsAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	bool bAI;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	bool bBoss = false;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	class UUPCharacterMovementComponent* UPCharacterMovementComponent;
	bool bIsJumping;
	int32 CurrentJumpCount;
	int32 MaxJumpCount;
public:
	friend class BTTask_Shot;
	AUntitledProjectCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void Jump() override;
	virtual void StopJumping() override;
	virtual void PostInitializeComponents() override;
	void PlayReloadMontage();
	void PlayFireMontage();
	void AIShoot();

protected:
	virtual void BeginPlay();

public:

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Bool for AnimBP to switch to another animation set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	bool bHasRifle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Weapon)
	bool bMelee = false;

	/** Setter to set the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetHasRifle(bool bNewHasRifle);

	/** Getter for the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetHasRifle();

	bool bPressedShinedownJump;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bBossKilled;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int Score;

	virtual void Landed(const FHitResult& Hit) override;
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();
protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SprintButtonPressed();
	void SprintButtonReleased();
	void CrouchButtonPressed();
	void CrouchButtonReleased();
	void DashButtonPressed();
	void DashButtonReleased();
	void JumpButtonPressed();
	void FireButtonPressed();
	void EquipButtonPressed();
	void FireButtonReleased();
	void ReloadButtonPressed();
	void MeleeButtonPressed();
	void SwapWeaponsButtonPressed();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamgeType, class AController* InstigatorController, AActor* DamageCauser);

	void PollInit();

	void SpawnDefaultWeapon();

private:

	UPROPERTY(VisibleAnywhere, Category = "Combat", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* CombatComponent;

	UPROPERTY(EditAnywhere, Category = "Combat", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* MeleeCollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	UPROPERTY()
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OverlappedWeapon(AWeapon* LastWeapon);

	UPROPERTY(EditAnywhere, Category = "Combat")
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* MeleeMontage;
	UPROPERTY(EditAnywhere, Category = "Combat")
	class USoundCue* DeathMusic;
	UPROPERTY(EditAnywhere, Category = "Combat")
	USoundCue* DeathGroan;

	/*
	* Health
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats", meta = (AllowPrivateAccess = "true"))
	float Health = 100.f;
	/*
	* Shield
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 75.f;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float Shield = 75.f;
	UPROPERTY()
	class AShinedownPlayerController* ShinedownPlayerController;

	UPROPERTY()
	class AShinedownPlayerState* ShinedownPlayerState;

	/*
	* AnimMomntages
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY(VisibleAnywhere, Category = "Combat", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bHasKeycard = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bOverlappingDoor = false;


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	FCollisionQueryParams GetIgnoreCharacterParams() const;
	UFUNCTION(BlueprintPure) FORCEINLINE UUPCharacterMovementComponent* GetUPCMC() const { return UPCharacterMovementComponent; }
	FORCEINLINE UCameraComponent* GetFPSCamera() const { return FirstPersonCameraComponent; }
	FORCEINLINE void SetCurrentJumpCount(int32 Value) { CurrentJumpCount = Value; }
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	AWeapon* GetEquippedWeapon();
	FORCEINLINE bool IsAI() const { return bAI; }
	FORCEINLINE UCombatComponent* GetCombat() const { return CombatComponent; }
	ECombatState GetCombatState() const;
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE UBoxComponent* GetMeleeCollisionBox() const { return MeleeCollisionBox; }
	FORCEINLINE void SetKeycard(bool bKeycard) { bHasKeycard = bKeycard; }
	FORCEINLINE bool HasKeycard() const { return bHasKeycard; }
	FORCEINLINE void SetOverlappingDoor(bool bDoor) { bOverlappingDoor = bDoor; }
	FORCEINLINE bool GetOverlappingDoor() const { return bOverlappingDoor; }


	FORCEINLINE UBuffComponent* GetBuffComponnt() const { return Buff; }

};

