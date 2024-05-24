// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Weapon.h"
#include "UntitledProjectCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "ShinedownPlayerController.h"
#include "ShinedownHUD.h"
#include "UPCharacterMovementComponent.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		InitializeCarriedAmmo();
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	PlayerController = PlayerController == nullptr ? Cast<AShinedownPlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed && CanFire())
	{
		Fire();	
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.75f;
		}
		if (Character)
		{
			Character->PlayFireMontage();
		}
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	if (Character == nullptr)
	{
		return;
	}
		AController* Controller = Character->GetController();
		if (Controller == nullptr)
		{
			return;
		}
		FVector Location;
		FRotator Rotation;
		Controller->GetPlayerViewPoint(Location, Rotation);

		FVector Start = Location;

		FVector End = Start + Rotation.Vector() * TRACE_LENGTH;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(EquippedWeapon);
		Params.AddIgnoredActor(GetOwner());
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			Params
		);
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
			HitTarget = End;
		}
		else
		{
			HitTarget = TraceHitResult.ImpactPoint;
			//DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
		}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr)
	{
		return;
	}
	PlayerController = PlayerController == nullptr ? Cast<AShinedownPlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		HUD = HUD == nullptr ? Cast<AShinedownHUD>(PlayerController->GetHUD()) : HUD;
		if (HUD)
		{
			FHUDPackage HUDPackage;
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
			}
			else
			{
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
			}
			//Cosshair spread
			FVector2D WalkSpeedRange(0.f, Character->GetUPCMC()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetUPCMC()->IsFalling())
			{
				CrosshaiInAirFactor = FMath::FInterpTo(CrosshaiInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshaiInAirFactor = FMath::FInterpTo(CrosshaiInAirFactor, 0.f, DeltaTime, 30.f);
			}
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshaiInAirFactor + CrosshairShootingFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}


void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr)
	{
		return;
	}
	
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr || Character == nullptr)
	{
		return;
	}
	/*
	UE_LOG
	(
		LogTemp,
		Warning,
		TEXT("Fire timer finished")
	);
	*/
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		if (EquippedWeapon)
		{
			EquippedWeapon->Fire(HitTarget);
		}
		StartFireTimer();
	}
}

void UCombatComponent::AIFire()
{
	if (CanFire())
	{

	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr)
	{
		return false;
	}

	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied && Character->bMelee == false
		
		
		
		;
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRLAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
}

bool UCombatComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);
	SetHUDCrosshairs(DeltaTime);
}

void UCombatComponent::Reload()
{
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}
	if (CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading)
	{
		int32 ReloadAmount = AmountToReload();
		if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
		{
			CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
			CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		}
		PlayerController = PlayerController == nullptr ? Cast<AShinedownPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
		}
		EquippedWeapon->AddAmmo(-ReloadAmount);
		HandleReload();
		UE_LOG(LogTemp, Warning, TEXT("Combat reload called"));
	}
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr)
	{
		return;
	}
		UE_LOG(LogTemp, Warning, TEXT("Now ECS_Unoccupied"));
	CombatState = ECombatState::ECS_Unoccupied;
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr)
	{
		return 0;
	}
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(AmountCarried, RoomInMag);
		return FMath::Clamp(RoomInMag, 0, Least);
	}

	return 0;
}

void UCombatComponent::SwapWeapons()
{
	AWeapon* AuxWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = AuxWeapon;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	UpdateCarriedAmmo();
	EquippedWeapon->SetHUDAmmo();

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::PerformMelee()
{
	TArray<AActor*> OverlappingActors;
	Character->GetMeleeCollisionBox()->GetOverlappingActors(OverlappingActors);
	UE_LOG(LogTemp, Warning, TEXT("Combat melee"));
	for (auto Actor : OverlappingActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("Melle for loop"));
		AUntitledProjectCharacter* OverlappedPawn = Cast<AUntitledProjectCharacter>(Actor);
		if (OverlappedPawn && OverlappedPawn->IsAI())
		{
			UE_LOG(LogTemp, Warning, TEXT("Trying to apply damage"));
			UGameplayStatics::ApplyDamage(OverlappedPawn, 500.f, PlayerController, GetOwner(), UDamageType::StaticClass());
			OverlappedPawn->GetMesh()->AddImpulse(-OverlappedPawn->GetActorForwardVector()*1000.f, NAME_None, true);
		}
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}
	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;


	Character->bHasRifle = true;
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	DropEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	//UpdateCarriedAmmo
	UpdateCarriedAmmo();

}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(WeaponToEquip);

	if (SecondaryWeapon == nullptr) return;
	SecondaryWeapon->SetOwner(Character);
}

void UCombatComponent::HandleReload()
{
	if (Character == nullptr)
	{
		return;
	}
	Character->PlayReloadMontage();
	CombatState = ECombatState::ECS_Reloading;
	UE_LOG(LogTemp, Warning, TEXT("Now ECS_Reloading"));


}


void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh1P() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* GripSocket = Character->GetMesh1P()->GetSocketByName(FName("GripPoint"));
	if (GripSocket)
	{
		GripSocket->AttachActor(ActorToAttach, Character->GetMesh1P());
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh1P() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh1P()->GetSocketByName(FName("BackpackSocket"));
	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh1P());
	}
}


