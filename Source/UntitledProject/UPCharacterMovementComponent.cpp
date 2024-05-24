// Fill out your copyright notice in the Description page of Project Settings.


#include "UPCharacterMovementComponent.h"

#include "UntitledProjectCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"



//Helper Macros
#if 0
float MacroDuration = 2.f;
#define SLOG(x) GEngine->AddOnScreenDebugMessage(-1, MacroDuration ? MacroDuration : -1.f, FColor::Yellow, x);
#define POINT(x, c) DrawDebugPoint(GetWorld(), x, 10, c, !MacroDuration, MacroDuration);
#define LINE(x1, x2, c) DrawDebugLine(GetWorld(), x1, x2, c, !MacroDuration, MacroDuration);
#define CAPSULE(x, c) DrawDebugCapsule(GetWorld(), x, CapHH(), CapR(), FQuat::Identity, c, !MacroDuration, MacroDuration);
#else
#define SLOG(x)
#define POINT(x, c)
#define LINE(x1, x2, c)
#define CAPSULE(x, c)
#endif


UUPCharacterMovementComponent::UUPCharacterMovementComponent() 
{
    NavAgentProps.bCanCrouch = true;
	bOrientRotationToMovement = false;
}

void UUPCharacterMovementComponent::InitializeComponent() 
{
    Super::InitializeComponent();

	UPCharacterOwner = Cast<AUntitledProjectCharacter>(GetOwner());
}

FNetworkPredictionData_Client* UUPCharacterMovementComponent::GetPredictionData_Client() const 
{
    check(PawnOwner != nullptr)

	if (ClientPredictionData == nullptr)
	{
		UUPCharacterMovementComponent* MutableThis = const_cast<UUPCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_UP(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f; 
	}
	return ClientPredictionData;
}

bool UUPCharacterMovementComponent::IsMovingOnGround() const 
{
    return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

bool UUPCharacterMovementComponent::CanCrouchInCurrentState() const 
{
    return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

float UUPCharacterMovementComponent::GetMaxSpeed() const 
{
    if (IsMovementMode(MOVE_Walking) && Safe_bWantsToSprint && !IsCrouching()) return MaxSprintSpeed;
	
	if (MovementMode != MOVE_Custom) return Super::GetMaxSpeed();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return MaxSlideSpeed;
	case CMOVE_WallRun:
		return MaxWallRunSpeed;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
		return -1.f;
	}
}

float UUPCharacterMovementComponent::GetMaxBrakingDeceleration() const 
{
    if (MovementMode != MOVE_Custom) return Super::GetMaxBrakingDeceleration();


	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return BrakingDecelerationSliding;
	case CMOVE_WallRun:
		return 0.f;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
		return -1.f;
	}
}

bool UUPCharacterMovementComponent::CanAttemptJump() const 
{
	return Super::CanAttemptJump() || IsWallRunning();
}

bool UUPCharacterMovementComponent::DoJump(bool bReplayingMoves) 
{
	bool bWasWallRunning = IsWallRunning();
	if (Super::DoJump(bReplayingMoves))
	{
		if (bWasWallRunning)
		{
			FVector Start = UpdatedComponent->GetComponentLocation();
			FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
			FVector End = Safe_bWallRunIsRight ? Start + CastDelta : Start - CastDelta;
			auto Params = UPCharacterOwner->GetIgnoreCharacterParams();
			FHitResult WallHit;
			GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
			Velocity += WallHit.Normal * WallJumpOffForce;
		}
		return true;
	}
	return false;
}

void UUPCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags) 
{
    Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToSprint = (Flags & FSavedMove_UP::FLAG_Sprint) != 0;
	Safe_bWantsToDash = (Flags & FSavedMove_UP::FLAG_Dash) !=0;
}

void UUPCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds) 
{
	//Slide
    if (MovementMode == MOVE_Walking && !bWantsToCrouch && Safe_bPrevWantsToCrouch)
	{
		if (CanSlide())
		{
			SetMovementMode(MOVE_Custom, CMOVE_Slide);
		}
	}

	if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
	{
		SetMovementMode(MOVE_Walking);
	}

	//Dash
	bool bAuthProxy = UPCharacterOwner->HasAuthority() && !UPCharacterOwner->IsLocallyControlled();
	if(Safe_bWantsToDash && CanDash())
	{
		if(!bAuthProxy || GetWorld()->GetTimeSeconds() - DashStartTime > AuthDashCooldownDuration)
		{
			EnterDash();
			Safe_bWantsToDash = false;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ClientTriedToCheat"));
		}
	}
	//Try Mantle
	if(UPCharacterOwner->bPressedShinedownJump)
	{
		if(TryMantle())
		{
			UPCharacterOwner->StopJumping();
		}
		else
		{
			UPCharacterOwner->bPressedShinedownJump = false;
			CharacterOwner->bPressedJump = true;
			CharacterOwner->CheckJumpInput(DeltaSeconds);
		}
	}

	// Transition Mantle
	if (Safe_bTransitionFinished)
	{
SLOG("Transition Finished")
		UE_LOG(LogTemp, Warning, TEXT("FINISHED RM"))

		if (IsValid(TransitionQueuedMontage))
		{
			SetMovementMode(MOVE_Flying);
			CharacterOwner->PlayAnimMontage(TransitionQueuedMontage, TransitionQueuedMontageSpeed);
			TransitionQueuedMontageSpeed = 0.f;
			TransitionQueuedMontage = nullptr;
		}
		else
		{
			SetMovementMode(MOVE_Walking);
		}

		Safe_bTransitionFinished = false;
	}

	//WallRun
	if(IsFalling())
	{
		TryWallRun();
	}

	
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UUPCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds) 
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);

	if (!HasAnimRootMotion() && Safe_bHadAnimRootMotion && IsMovementMode(MOVE_Flying))
	{
		UE_LOG(LogTemp, Warning, TEXT("Ending Anim Root Motion"))
		SetMovementMode(MOVE_Walking);
	}

	if (GetRootMotionSourceByID(TransitionRMS_ID) && GetRootMotionSourceByID(TransitionRMS_ID)->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
	{
		RemoveRootMotionSourceByID(TransitionRMS_ID);
		Safe_bTransitionFinished = true;
	}
	Safe_bHadAnimRootMotion = HasAnimRootMotion();
}

void UUPCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) 
{
    Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	Safe_bPrevWantsToCrouch = bWantsToCrouch;
}

void UUPCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations) 
{
    Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		PhysSlide(deltaTime, Iterations);
		break;
	case CMOVE_WallRun:
		PhysWallRun(deltaTime, Iterations);
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
	}
}

void UUPCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) 
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Slide) ExitSlide();

	if (IsCustomMovementMode(CMOVE_Slide)) EnterSlide(PreviousMovementMode, (ECustomMovementMode)PreviousCustomMode);

	if(IsWallRunning())
	{
		FVector Start = UpdatedComponent->GetComponentLocation();
		FVector End = Start + UpdatedComponent->GetRightVector() * CapR() * 2;
		auto Params = UPCharacterOwner->GetIgnoreCharacterParams();
		FHitResult WallHit;
		Safe_bWallRunIsRight = GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);	
	}
}

void UUPCharacterMovementComponent::EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode) 
{
    bWantsToCrouch = true;
	bOrientRotationToMovement = false;
	Velocity += Velocity.GetSafeNormal2D() * SlideEnterImpulse;

	FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, true, NULL);
	
	SetMovementMode(MOVE_Custom, CMOVE_Slide);

	bIsSliding = true;
}

void UUPCharacterMovementComponent::ExitSlide() 
{
    bWantsToCrouch = false;
	bOrientRotationToMovement = false;
	bIsSliding = false;
}

bool UUPCharacterMovementComponent::CanSlide() const
{
    FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.5f * FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");
	bool bValidSurface = GetWorld()->LineTraceTestByProfile(Start, End, ProfileName, UPCharacterOwner->GetIgnoreCharacterParams());
	bool bEnoughSpeed = Velocity.SizeSquared() > pow(MinSlideSpeed, 2);
	
	return bValidSurface && bEnoughSpeed;
}

void UUPCharacterMovementComponent::PhysSlide(float deltaTime, int32 Iterations) 
{
    if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	
	if (!CanSlide())
	{
		SetMovementMode(MOVE_Walking);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	while ( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)) )
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;

		FVector SlopeForce = CurrentFloor.HitResult.Normal;
		SlopeForce.Z = 0.f;
		Velocity += SlopeForce * SlideGravityForce * deltaTime;
		
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector().GetSafeNormal2D());

		CalcVelocity(timeTick, GroundFriction * SlideFrictionFactor, false, GetMaxBrakingDeceleration());
		
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;
		bool bFloorWalkable = CurrentFloor.IsWalkableFloor();

		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if ( IsFalling() )
			{
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f,ActualDist/DesiredDist));
				}
				StartNewPhysics(remainingTime,Iterations);
				return;
			}
			else if ( IsSwimming() )
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}


		const bool bCheckLedges = !CanWalkOffLedges();
		if ( bCheckLedges && !CurrentFloor.IsWalkableFloor() )
		{
			const FVector GravDir = FVector(0.f,0.f,-1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir);
			if ( !NewDelta.IsZero() )
			{
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				bTriedLedgeMove = true;

				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;
				continue;
			}
			else
			{
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ( (bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;

				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			if ( IsSwimming() )
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;
			}
		}
		
		if (IsMovingOnGround() && bFloorWalkable)
		{
			if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}


	FHitResult Hit;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(Velocity.GetSafeNormal2D(), FVector::UpVector).ToQuat();
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, Hit);
}

bool UUPCharacterMovementComponent::CanDash() const
{
	return !IsCrouching();
}

void UUPCharacterMovementComponent::EnterDash() 
{
	DashStartTime = GetWorld()->GetTimeSeconds();

	FVector DashDirection = (Acceleration.IsNearlyZero() ? UpdatedComponent->GetForwardVector() : Acceleration).GetSafeNormal2D();
	Velocity += DashImpulse * (DashDirection + FVector::UpVector * .1);

	FQuat NewRotation = FRotationMatrix::MakeFromXZ(DashDirection, FVector::UpVector).ToQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, Hit);

	SetMovementMode(MOVE_Falling);
	DashStartDelegate.Broadcast();
}

void UUPCharacterMovementComponent::OnDashCooldownFinished() 
{
	Safe_bWantsToDash = true;
}

bool UUPCharacterMovementComponent::TryMantle() 
{
	if(!(IsMovementMode(MOVE_Walking) && !!IsCrouching()) && !IsMovementMode(MOVE_Falling))
	{
		return false;
	}

	// Helper Variables
	FVector BaseLoc = UpdatedComponent->GetComponentLocation() + FVector::DownVector * CapHH();
	FVector Fwd = UpdatedComponent->GetForwardVector().GetSafeNormal2D();
	auto Params = UPCharacterOwner->GetIgnoreCharacterParams();
	float MaxHeight = CapHH() * 2+ MantleReachHeight;
	float CosMMWSA = FMath::Cos(FMath::DegreesToRadians(MantleMinWallSteepnessAngle));
	float CosMMSA = FMath::Cos(FMath::DegreesToRadians(MantleMaxSurfaceAngle));
	float CosMMAA = FMath::Cos(FMath::DegreesToRadians(MantleMaxAlignmentAngle));


//SLOG("Starting Mantle Attempt") pt debug nu mai treve

	//Check Front Face castez niste linii debug lines in fata caracterului sa vad daca are de ce sa se prinda(ledguri)
	// The '|' operator calculates the dot product between 2 vectors :), mi-o luat 20 de min sa imi dau seama ce face, super tare frate
	FHitResult FrontHit;
	float CheckDistance = FMath::Clamp(Velocity | Fwd, CapR() + 30, MantleMaxDistance);
	FVector FrontStart = BaseLoc + FVector::UpVector * (MaxStepHeight - 1);
	for (int i = 0; i < 6; i++)
	{
LINE(FrontStart, FrontStart + Fwd * CheckDistance, FColor::Red)
		if (GetWorld()->LineTraceSingleByProfile(FrontHit, FrontStart, FrontStart + Fwd * CheckDistance, "BlockAll", Params)) break;
		FrontStart += FVector::UpVector * (2.f * CapHH() - (MaxStepHeight - 1)) / 5;
	}
	if (!FrontHit.IsValidBlockingHit()) 
	{
		return false;
	}
	float CosWallSteepnessAngle = FrontHit.Normal | FVector::UpVector;
	if (FMath::Abs(CosWallSteepnessAngle) > CosMMWSA || (Fwd | -FrontHit.Normal) < CosMMAA) 
	{
		return false;
	}

//POINT(FrontHit.Location, FColor::Red);

	// Check Top Surface (daca nu e la unghi prea abrupt)

	TArray<FHitResult> HeightHits;
	FHitResult SurfaceHit;
	FVector WallUp = FVector::VectorPlaneProject(FVector::UpVector, FrontHit.Normal).GetSafeNormal();
	float WallCos = FVector::UpVector | FrontHit.Normal;
	float WallSin = FMath::Sqrt(1 - WallCos * WallCos);
	FVector TraceStart = FrontHit.Location + Fwd + WallUp * (MaxHeight - (MaxStepHeight - 1)) / WallSin;
//LINE(TraceStart, FrontHit.Location + Fwd, FColor::Orange) a;te linii de debug
	if (!GetWorld()->LineTraceMultiByProfile(HeightHits, TraceStart, FrontHit.Location + Fwd, "BlockAll", Params)) return false;
	for (const FHitResult& Hit : HeightHits)
	{
		if (Hit.IsValidBlockingHit())
		{
			SurfaceHit = Hit;
			break;
		}
	}
	if (!SurfaceHit.IsValidBlockingHit() || (SurfaceHit.Normal | FVector::UpVector) < CosMMSA) return false;
	float Height = (SurfaceHit.Location - BaseLoc) | FVector::UpVector;

SLOG(FString::Printf(TEXT("Height: %f"), Height))
POINT(SurfaceHit.Location, FColor::Blue);
	
	if (Height > MaxHeight) return false;

	// Check Clearance(daca are loc caracteru pe perete sau pe ce se urca)
	float SurfaceCos = FVector::UpVector | SurfaceHit.Normal;
	float SurfaceSin = FMath::Sqrt(1 - SurfaceCos * SurfaceCos);												//Adds to see when/if we have a slope if it is ok
	FVector ClearCapLoc = SurfaceHit.Location + Fwd * CapR() + FVector::UpVector * (CapHH() + 1 + CapR() * 2 * SurfaceSin);
	FCollisionShape CapShape = FCollisionShape::MakeCapsule(CapR(), CapHH());
/*	if (GetWorld()->OverlapAnyTestByProfile(ClearCapLoc, FQuat::Identity, "BlockAll", CapShape, Params))
	{
CAPSULE(ClearCapLoc, FColor::Red)
		return false;
	}
	else
	{
CAPSULE(ClearCapLoc, FColor::Green)
	}
	SLOG("Can Mantle")	aci tot pt debug desenam o sfera in functie de posibilitatea caracterului de a sta pe ledge			*/
	
	// Mantle Selection
	FVector ShortMantleTarget = GetMantleStartLocation(FrontHit, SurfaceHit, false);
	FVector TallMantleTarget = GetMantleStartLocation(FrontHit, SurfaceHit, true);
	
	bool bTallMantle = false;
	if (IsMovementMode(MOVE_Walking) && Height > CapHH() * 2)
		bTallMantle = true;
	else if (IsMovementMode(MOVE_Falling) && (Velocity | FVector::UpVector) < 0)
	{
		if (!GetWorld()->OverlapAnyTestByProfile(TallMantleTarget, FQuat::Identity, "BlockAll", CapShape, Params))
			bTallMantle = true;
	}
	FVector TransitionTarget = bTallMantle ? TallMantleTarget : ShortMantleTarget;
CAPSULE(TransitionTarget, FColor::Yellow)

	// Perform Transition to Mantle
CAPSULE(UpdatedComponent->GetComponentLocation(), FColor::Red)
	UPCharacterOwner->SetCurrentJumpCount(0);
	float UpSpeed = Velocity | FVector::UpVector;
	float TransDistance = FVector::Dist(TransitionTarget, UpdatedComponent->GetComponentLocation());

	TransitionQueuedMontageSpeed = FMath::GetMappedRangeValueClamped(FVector2D(-500, 750), FVector2D(.9f, 1.2f), UpSpeed);
	TransitionRMS.Reset();
	TransitionRMS = MakeShared<FRootMotionSource_MoveToForce>();
	TransitionRMS->AccumulateMode = ERootMotionAccumulateMode::Override;
	
	TransitionRMS->Duration = FMath::Clamp(TransDistance / 500.f, .1f, .25f);
SLOG(FString::Printf(TEXT("Duration: %f"), TransitionRMS->Duration))
	TransitionRMS->StartLocation = UpdatedComponent->GetComponentLocation();
	TransitionRMS->TargetLocation = TransitionTarget;

	// Apply Transition Root Motion Source
	Velocity = FVector::ZeroVector;
	SetMovementMode(MOVE_Falling);
	TransitionRMS_ID = ApplyRootMotionSource(TransitionRMS);

	// Animations
	if (bTallMantle)
	{
		TransitionQueuedMontage = TallMantleMontage;
		CharacterOwner->PlayAnimMontage(TransitionTallMantleMontage, 1 / TransitionRMS->Duration);
	}
	else
	{
		TransitionQueuedMontage = ShortMantleMontage;
		CharacterOwner->PlayAnimMontage(TransitionShortMantleMontage, 1 / TransitionRMS->Duration);
	}

	return true;
}

FVector UUPCharacterMovementComponent::GetMantleStartLocation(FHitResult FrontHit, FHitResult SurfaceHit, bool bTallMantle) const
{
	float CosWallSteepnessAngle = FrontHit.Normal | FVector::UpVector;
	float DownDistance = bTallMantle ? CapHH() * 2.f : MaxStepHeight - 1;
	FVector EdgeTangent = FVector::CrossProduct(SurfaceHit.Normal, FrontHit.Normal).GetSafeNormal();

	FVector MantleStart = SurfaceHit.Location;
	MantleStart += FrontHit.Normal.GetSafeNormal2D() * (2.f + CapR());
	MantleStart += UpdatedComponent->GetForwardVector().GetSafeNormal2D().ProjectOnTo(EdgeTangent) * CapR() * .3f;
	MantleStart += FVector::UpVector * CapHH();
	MantleStart += FVector::DownVector * DownDistance;
	MantleStart += FrontHit.Normal.GetSafeNormal2D() * CosWallSteepnessAngle * DownDistance;

	return MantleStart;
}

bool UUPCharacterMovementComponent::TryWallRun() 
{
	if (!IsFalling()) return false;
	if (Velocity.SizeSquared2D() < pow(MinWallRunSpeed, 2)) return false;

	//if (Velocity.Z < -MaxVerticalWallRunSpeed) return false;

	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector LeftEnd = Start - UpdatedComponent->GetRightVector() * CapR() * 2;
	FVector RightEnd = Start + UpdatedComponent->GetRightVector() * CapR() * 2;
	auto Params = UPCharacterOwner->GetIgnoreCharacterParams();

	FHitResult FloorHit, WallHit;

	// Check Player Height
	if (GetWorld()->LineTraceSingleByProfile(FloorHit, Start, Start + FVector::DownVector * (CapHH() + MinWallRunHeight), "BlockAll", Params))
	{
		return false;
	}
	
	// Left Cast
	GetWorld()->LineTraceSingleByProfile(WallHit, Start, LeftEnd, "BlockAll", Params);
	if (WallHit.IsValidBlockingHit() && (Velocity | WallHit.Normal) < 0)
	{
		Safe_bWallRunIsRight = false;
	}
	// Right Cast
	else
	{
		GetWorld()->LineTraceSingleByProfile(WallHit, Start, RightEnd, "BlockAll", Params);
		if (WallHit.IsValidBlockingHit() && (Velocity | WallHit.Normal) < 0)
		{
			Safe_bWallRunIsRight = true;
		}
		else
		{
			return false;
		}
	}
	FVector ProjectedVelocity = FVector::VectorPlaneProject(Velocity, WallHit.Normal);
	if (ProjectedVelocity.SizeSquared2D() < pow(MinWallRunSpeed, 2)) return false;
	
	// Passed all conditions
	Velocity = ProjectedVelocity;
	Velocity.Z = FMath::Clamp(Velocity.Z, 0.f, MaxVerticalWallRunSpeed);
	SetMovementMode(MOVE_Custom, CMOVE_WallRun);
	UPCharacterOwner->SetCurrentJumpCount(0);

//SLOG("Starting WallRun")
	return true;
}

void UUPCharacterMovementComponent::PhysWallRun(float deltaTime, int32 Iterations) 
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity()))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}
	
	bJustTeleported = false;
	float remainingTime = deltaTime;
	// Perform the move
	while ( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController) )
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		
		FVector Start = UpdatedComponent->GetComponentLocation();
		FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
		FVector End = Safe_bWallRunIsRight ? Start + CastDelta : Start - CastDelta;
		auto Params = UPCharacterOwner->GetIgnoreCharacterParams();
		float SinPullAwayAngle = FMath::Sin(FMath::DegreesToRadians(WallRunPullAwayAngle));
		FHitResult WallHit;
		GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
		bool bWantsToPullAway = WallHit.IsValidBlockingHit() && !Acceleration.IsNearlyZero() && (Acceleration.GetSafeNormal() | WallHit.Normal) > SinPullAwayAngle;
		if (!WallHit.IsValidBlockingHit() || bWantsToPullAway)
		{
			SetMovementMode(MOVE_Falling);
			StartNewPhysics(remainingTime, Iterations);
			return;
		}
		// Clamp Acceleration
		Acceleration = FVector::VectorPlaneProject(Acceleration, WallHit.Normal);
		Acceleration.Z = 0.f;
		// Apply acceleration
		CalcVelocity(timeTick, 0.f, false, GetMaxBrakingDeceleration());
		Velocity = FVector::VectorPlaneProject(Velocity, WallHit.Normal);
		float TangentAccel = Acceleration.GetSafeNormal() | Velocity.GetSafeNormal2D();
		bool bVelUp = Velocity.Z > 0.f;
		Velocity.Z += GetGravityZ() * WallRunGravityScaleCurve->GetFloatValue(bVelUp ? 0.f : TangentAccel) * timeTick;
		if (Velocity.SizeSquared2D() < pow(MinWallRunSpeed, 2) || Velocity.Z < -MaxVerticalWallRunSpeed)
		{
			SetMovementMode(MOVE_Falling);
			StartNewPhysics(remainingTime, Iterations);
			return;
		}
		
		// Compute move parameters
		const FVector Delta = timeTick * Velocity; // dx = v * dt
		const bool bZeroDelta = Delta.IsNearlyZero();
		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			FHitResult Hit;
			SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
			FVector WallAttractionDelta = -WallHit.Normal * WallAttractionForce * timeTick;
			SafeMoveUpdatedComponent(WallAttractionDelta, UpdatedComponent->GetComponentQuat(), true, Hit);
		}
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick; // v = dx / dt
	}

	
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
	FVector End = Safe_bWallRunIsRight ? Start + CastDelta : Start - CastDelta;
	auto Params = UPCharacterOwner->GetIgnoreCharacterParams();
	FHitResult FloorHit, WallHit;
	GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
	GetWorld()->LineTraceSingleByProfile(FloorHit, Start, Start + FVector::DownVector * (CapHH() + MinWallRunHeight * .5f), "BlockAll", Params);
	if (FloorHit.IsValidBlockingHit() || !WallHit.IsValidBlockingHit() || Velocity.SizeSquared2D() < pow(MinWallRunSpeed, 2))
	{
		SetMovementMode(MOVE_Falling);
	}
}

float UUPCharacterMovementComponent::CapR() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UUPCharacterMovementComponent::CapHH() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void UUPCharacterMovementComponent::SprintPressed() 
{
    Safe_bWantsToSprint = true;
}

void UUPCharacterMovementComponent::SprintReleased() 
{
    Safe_bWantsToSprint = false;
}

void UUPCharacterMovementComponent::CrouchPressed() 
{
    bWantsToCrouch = !bWantsToCrouch;
	if(Safe_bWantsToSprint && bWantsToCrouch && IsMovingOnGround())
	{
		EnterSlide(MovementMode, (ECustomMovementMode)CustomMovementMode);
	}
}

void UUPCharacterMovementComponent::CrouchReleased() 
{
}

void UUPCharacterMovementComponent::DashPressed() 
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if(CurrentTime - DashStartTime >= DashCooldownDuration)
	{
		Safe_bWantsToDash = true;
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_DashCooldown, this,
		 &UUPCharacterMovementComponent::OnDashCooldownFinished, DashCooldownDuration - (CurrentTime - DashStartTime)
		);
	}
}

void UUPCharacterMovementComponent::DashReleased() 
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_DashCooldown);
	Safe_bWantsToDash = false;
}

bool UUPCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
    return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

bool UUPCharacterMovementComponent::IsMovementMode(EMovementMode InMovementMode) const
{
    return InMovementMode == MovementMode;
}

bool UUPCharacterMovementComponent::IsCrouching() const 
{
    return UPCharacterOwner && UPCharacterOwner->bIsCrouched;
}

UUPCharacterMovementComponent::FSavedMove_UP::FSavedMove_UP() 
{
    Saved_bWantsToSprint=0;
}

bool UUPCharacterMovementComponent::FSavedMove_UP::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const 
{
    const FSavedMove_UP* NewTurnMove = static_cast<FSavedMove_UP*>(NewMove.Get());

	if (Saved_bWantsToSprint != NewTurnMove->Saved_bWantsToSprint)
	{
		return false;
	}

	if(Saved_bWantsToDash != NewTurnMove->Saved_bWantsToDash)
	{
		return false;
	}

	if(Saved_bWallRunIsRight != NewTurnMove->Saved_bWallRunIsRight)
	{
		return false;
	}
	
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UUPCharacterMovementComponent::FSavedMove_UP::Clear() 
{
    FSavedMove_Character::Clear();

	Saved_bWantsToSprint = 0;

	Saved_bWantsToDash = 0;

	Saved_bPrevWantsToCrouch = 0;

	Saved_bPressedShinedownJump = 0;
	Saved_bHadAnimRootMotion = 0;
	Saved_bTransitionFinished = 0;

	Saved_bWallRunIsRight = 0;
}

uint8 UUPCharacterMovementComponent::FSavedMove_UP::GetCompressedFlags() const 
{
    uint8 Result = FSavedMove_Character::GetCompressedFlags();

	if(Saved_bWantsToSprint) Result |= FLAG_Sprint;
	if(Saved_bWantsToDash) Result |= FLAG_Dash;
	if(Saved_bPressedShinedownJump) Result |= FLAG_JumpPressed;

	return Result;
}

void UUPCharacterMovementComponent::FSavedMove_UP::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) 
{
    FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
	
	const UUPCharacterMovementComponent* CharacterMovement = Cast<UUPCharacterMovementComponent>(C->GetCharacterMovement());

	Saved_bWantsToSprint = CharacterMovement->Safe_bWantsToSprint;
	Saved_bPrevWantsToCrouch = CharacterMovement->Safe_bPrevWantsToCrouch;
	Saved_bWantsToDash = CharacterMovement->Safe_bWantsToDash;

	Saved_bPressedShinedownJump = CharacterMovement->UPCharacterOwner->bPressedShinedownJump;
	Saved_bHadAnimRootMotion = CharacterMovement->Safe_bHadAnimRootMotion;
	Saved_bTransitionFinished = CharacterMovement->Safe_bTransitionFinished;

	Saved_bWallRunIsRight = CharacterMovement->Safe_bWallRunIsRight;

}

void UUPCharacterMovementComponent::FSavedMove_UP::PrepMoveFor(ACharacter* C) 
{
    FSavedMove_Character::PrepMoveFor(C);

	UUPCharacterMovementComponent* CharacterMovement = Cast<UUPCharacterMovementComponent>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
	CharacterMovement->Safe_bPrevWantsToCrouch = Saved_bPrevWantsToCrouch;
	CharacterMovement->Safe_bWantsToDash = Saved_bWantsToDash;

	CharacterMovement->UPCharacterOwner->bPressedShinedownJump = Saved_bPressedShinedownJump;
	CharacterMovement->Safe_bHadAnimRootMotion = Saved_bHadAnimRootMotion;
	CharacterMovement->Safe_bTransitionFinished = Saved_bTransitionFinished;

	CharacterMovement->Safe_bWallRunIsRight = Saved_bWallRunIsRight;
}

UUPCharacterMovementComponent::FNetworkPredictionData_Client_UP::FNetworkPredictionData_Client_UP(const UCharacterMovementComponent& ClientMovement) 
: Super(ClientMovement)
{
}

FSavedMovePtr UUPCharacterMovementComponent::FNetworkPredictionData_Client_UP::AllocateNewMove() 
{
    return FSavedMovePtr(new FSavedMove_UP());
}
