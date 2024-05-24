// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "UntitledProject.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UPCharacterMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDashStartDelegate);

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None			UMETA(Hidden),
	CMOVE_Slide			UMETA(DisplayName = "Slide"),
	CMOVE_WallRun		UMETA(DisplayName = "Wall Run"),
	CMOVE_MAX			UMETA(Hidden),
};

UCLASS()
class UNTITLEDPROJECT_API UUPCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	class FSavedMove_UP : public FSavedMove_Character
	{
	public:
		enum CompressedFlags
		{
			FLAG_Sprint			= 0x10,
			FLAG_Dash			= 0x20,
			FLAG_Custom_2		= 0x40,
			FLAG_Custom_3		= 0x80,
		};
		
		// Flags
		uint8 Saved_bWantsToSprint:1;
		uint8 Saved_bWantsToDash:1;
		uint8 Saved_bPressedShinedownJump:1;

		// Other Variables
		uint8 Saved_bPrevWantsToCrouch:1;
		uint8 Saved_bHadAnimRootMotion:1;
		uint8 Saved_bTransitionFinished:1;
		uint8 Saved_bWallRunIsRight:1;


		FSavedMove_UP();

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_UP : public FNetworkPredictionData_Client_Character
	{
		public:
		FNetworkPredictionData_Client_UP(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	// Parameters
	UPROPERTY(EditDefaultsOnly) float MaxSprintSpeed=750.f;

		//Slide
		UPROPERTY(EditDefaultsOnly) float MinSlideSpeed=400.f;
		UPROPERTY(EditDefaultsOnly) float MaxSlideSpeed=400.f;
		UPROPERTY(EditDefaultsOnly) float SlideEnterImpulse=400.f;
		UPROPERTY(EditDefaultsOnly) float SlideGravityForce=4000.f;
		UPROPERTY(EditDefaultsOnly) float SlideFrictionFactor=.06f;
		UPROPERTY(EditDefaultsOnly) float BrakingDecelerationSliding=1000.f;

		//Dash
		UPROPERTY(EditDefaultsOnly) float DashImpulse=1000.f;
		UPROPERTY(EditDefaultsOnly) float DashCooldownDuration=1.f;
		UPROPERTY(EditDefaultsOnly) float AuthDashCooldownDuration=.9f;

		//Mantle
		UPROPERTY(EditDefaultsOnly) float MantleMaxDistance = 200.f;
    	UPROPERTY(EditDefaultsOnly) float MantleReachHeight = 50.f;
    	UPROPERTY(EditDefaultsOnly) float MinMantleDepth = 30.f;
    	UPROPERTY(EditDefaultsOnly) float MantleMinWallSteepnessAngle = 75.f;
    	UPROPERTY(EditDefaultsOnly) float MantleMaxSurfaceAngle = 40.f;
    	UPROPERTY(EditDefaultsOnly) float MantleMaxAlignmentAngle = 45.f;
		UPROPERTY(EditDefaultsOnly) UAnimMontage* TallMantleMontage;
		UPROPERTY(EditDefaultsOnly) UAnimMontage* TransitionTallMantleMontage;
		UPROPERTY(EditDefaultsOnly) UAnimMontage* ProxyTallMantleMontage;
		UPROPERTY(EditDefaultsOnly) UAnimMontage* ShortMantleMontage;
		UPROPERTY(EditDefaultsOnly) UAnimMontage* TransitionShortMantleMontage;
		UPROPERTY(EditDefaultsOnly) UAnimMontage* ProxyShortMantleMontage;

		// Wall Run
		UPROPERTY(EditDefaultsOnly) float MinWallRunSpeed=200.f;
		UPROPERTY(EditDefaultsOnly) float MaxWallRunSpeed=800.f;
		UPROPERTY(EditDefaultsOnly) float MaxVerticalWallRunSpeed=200.f;
		UPROPERTY(EditDefaultsOnly) float WallRunPullAwayAngle=75.f;
		UPROPERTY(EditDefaultsOnly) float WallAttractionForce = 200.f;
		UPROPERTY(EditDefaultsOnly) float MinWallRunHeight=50.f;
		UPROPERTY(EditDefaultsOnly) UCurveFloat* WallRunGravityScaleCurve;
		UPROPERTY(EditDefaultsOnly) float WallJumpOffForce = 300.f;

	UPROPERTY(Transient)
	class AUntitledProjectCharacter* UPCharacterOwner;
	
	//Flags
	bool Safe_bWantsToSprint;
	bool Safe_bPrevWantsToCrouch;
	bool Safe_bWantsToDash;

	bool Safe_bHadAnimRootMotion;
	bool bIsSliding;
	float DashStartTime;

	FTimerHandle TimerHandle_DashCooldown;

	//Transition variables for transitioning into the mantle
	bool Safe_bTransitionFinished;
	TSharedPtr<FRootMotionSource_MoveToForce> TransitionRMS;
	UPROPERTY(Transient) UAnimMontage* TransitionQueuedMontage;
	float TransitionQueuedMontageSpeed;
	int TransitionRMS_ID;
	
	bool Safe_bWallRunIsRight;


public:
	UUPCharacterMovementComponent();
	friend class AUntitledProjectCharacter;

//Delegates
public:
	UPROPERTY(BlueprintAssignable) FDashStartDelegate DashStartDelegate;
protected:
	virtual void InitializeComponent() override;
	
public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;

	virtual bool CanAttemptJump() const override;
	virtual bool DoJump(bool bReplayingMoves) override;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
public:
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
protected:
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	// Slide
private:
	void EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode);
	bool CanSlide() const;
	void PhysSlide(float deltaTime, int32 Iterations);

	//Dash
private:
	bool CanDash() const;
	void EnterDash();
	void OnDashCooldownFinished();

	//Mantle
private:
	bool TryMantle();
	FVector GetMantleStartLocation(FHitResult FrontHit, FHitResult SurfaceHit, bool bTallMantle) const;

	//Wall Run
private:
	bool TryWallRun();
	void PhysWallRun(float deltaTime, int32 Iterations);

	// Helpers
private:
	float CapR() const;  //GetCapsuleScaledRadius
	float CapHH() const; //GetCapsuleScaledHalfHeight

	// Interface
public:
	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();
	void ExitSlide();

	UFUNCTION(BlueprintCallable) void CrouchPressed();
	UFUNCTION(BlueprintCallable) void CrouchReleased();

	UFUNCTION(BlueprintCallable) void DashPressed();
	UFUNCTION(BlueprintCallable) void DashReleased();

	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	UFUNCTION(BlueprintPure) bool IsMovementMode(EMovementMode InMovementMode) const;

	UFUNCTION(BlueprintPure) bool IsWallRunning() const { return IsCustomMovementMode(CMOVE_WallRun); }
	UFUNCTION(BlueprintPure) bool WallRunningIsRight() const { return Safe_bWallRunIsRight; }


	virtual bool IsCrouching() const override;

//Public Getters
public:
	FORCEINLINE bool GetSliding() const { return bIsSliding; }
};
