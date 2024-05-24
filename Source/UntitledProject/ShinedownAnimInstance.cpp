// Fill out your copyright notice in the Description page of Project Settings.


#include "ShinedownAnimInstance.h"
#include "UntitledProjectCharacter.h"
#include "UPCharacterMovementComponent.h"
#include "Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "CombatState.h"


void UShinedownAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShinedownCharacter = Cast<AUntitledProjectCharacter>(TryGetPawnOwner());
}

void UShinedownAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	ShinedownCharacter = ShinedownCharacter == nullptr ? Cast<AUntitledProjectCharacter>(TryGetPawnOwner()) : ShinedownCharacter;
	if (ShinedownCharacter == nullptr)
	{
		return;
	}
	FVector Velocity = ShinedownCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = ShinedownCharacter->GetUPCMC()->IsFalling();

	bIsAccelerating = ShinedownCharacter->GetUPCMC()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = ShinedownCharacter->IsWeaponEquipped();
	EquippedWeapon = ShinedownCharacter->GetEquippedWeapon();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && ShinedownCharacter->GetMesh1P())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPos;
		FRotator OutRot;
		
		ShinedownCharacter->GetMesh1P()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPos, OutRot);
		LeftHandTransform.SetLocation(OutPos);
		LeftHandTransform.SetRotation(FQuat(OutRot));
	}
	bUseFABRIK = ShinedownCharacter->GetCombatState() != ECombatState::ECS_Reloading && ShinedownCharacter->bMelee == false;
}
