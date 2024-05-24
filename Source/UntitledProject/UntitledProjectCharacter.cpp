// Copyright Epic Games, Inc. All Rights Reserved.

#include "UntitledProjectCharacter.h"
#include "UntitledProjectProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UPCharacterMovementComponent.h"
#include "CombatComponent.h"
#include "Weapon.h"
#include "ShinedownPlayerController.h"
#include "WeaponTypes.h"
#include "Animation/AnimInstance.h"
#include "ShinedownAnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "ShinedownPlayerState.h"
#include "BuffComponent.h"
#include "Components/BoxComponent.h"
#include "ShinedownHUD.h"
#include "Blueprint/UserWidget.h"
#include "ShinedownGameMode.h"
#include "Sound/SoundCue.h"


AUntitledProjectCharacter::AUntitledProjectCharacter(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer.SetDefaultSubobjectClass<UUPCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	UPCharacterMovementComponent = Cast<UUPCharacterMovementComponent>(GetCharacterMovement());
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	MeleeCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("MeleeCollision"));
	MeleeCollisionBox->SetupAttachment(Mesh1P);
	MeleeCollisionBox->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat Component"));
	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("Buff Component"));

	bIsJumping = false;
	CurrentJumpCount = 0;
	MaxJumpCount = 2;



	PollInit();
}

void AUntitledProjectCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(CombatComponent)
	{
		CombatComponent->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
	}
}

void AUntitledProjectCharacter::PlayReloadMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh1P()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
				SectionName = FName("Rifle");
				break;
		case EWeaponType::EWT_RocketLauncher:
				SectionName = FName("Rifle");
				break;
		case EWeaponType::EWT_Pistol:
				SectionName = FName("Pistol");
				break;
		case EWeaponType::EWT_SMG:
				SectionName = FName("Rifle");
				break;
		case EWeaponType::EWT_Shotgun:
				SectionName = FName("Rifle");
				break;
		
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AUntitledProjectCharacter::PlayFireMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh1P()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
	}
}

void AUntitledProjectCharacter::AIShoot()
{
	if (CombatComponent == nullptr)
	{
		return;
	}
	CombatComponent->Fire();
}

void AUntitledProjectCharacter::Jump() 
{
	Super::Jump();

	bPressedShinedownJump = true;

	bPressedJump = false;
}

void AUntitledProjectCharacter::StopJumping() 
{
	Super::StopJumping();

	bPressedShinedownJump = false;
}

void AUntitledProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	SpawnDefaultWeapon();
	UpdateHUDAmmo();

	UpdateHUDHealth();
	UpdateHUDShield();
	OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);

}

//////////////////////////////////////////////////////////////////////////// Input

void AUntitledProjectCharacter::OverlappedWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon && HasAuthority())
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void AUntitledProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::JumpButtonPressed);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUntitledProjectCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUntitledProjectCharacter::Look);

		//Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ThisClass::SprintButtonPressed);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::SprintButtonReleased);

		//Crouch/Slide
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ThisClass::CrouchButtonReleased);

		//Dash
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &ThisClass::DashButtonPressed);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Completed, this, &ThisClass::DashButtonReleased);

		//Equip weapon
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ThisClass::EquipButtonPressed);

		//Fire Weapon
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ThisClass::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::FireButtonReleased);
		
		//Reload Weapon
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ThisClass::ReloadButtonPressed);

		//SwapWeapons
		EnhancedInputComponent->BindAction(SwapWeaponsAction, ETriggerEvent::Triggered, this, &ThisClass::SwapWeaponsButtonPressed);

		EnhancedInputComponent->BindAction(MeleeAction, ETriggerEvent::Triggered, this, &ThisClass::MeleeButtonPressed);
	}
}

FCollisionQueryParams AUntitledProjectCharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;
	
	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}


void AUntitledProjectCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AUntitledProjectCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AUntitledProjectCharacter::SprintButtonPressed() 
{
	if(UPCharacterMovementComponent == nullptr) return;
	UPCharacterMovementComponent->SprintPressed();
}

void AUntitledProjectCharacter::SprintButtonReleased() 
{
	if(UPCharacterMovementComponent == nullptr) return;
	UPCharacterMovementComponent->SprintReleased();
}

void AUntitledProjectCharacter::CrouchButtonPressed() 
{
	if(UPCharacterMovementComponent == nullptr) return;

	UPCharacterMovementComponent->CrouchPressed();
}

void AUntitledProjectCharacter::CrouchButtonReleased() 
{
	if(UPCharacterMovementComponent == nullptr) return;

	UPCharacterMovementComponent->CrouchReleased();
}

void AUntitledProjectCharacter::DashButtonPressed() 
{
	if(UPCharacterMovementComponent == nullptr) return;
	UPCharacterMovementComponent->DashPressed();
}

void AUntitledProjectCharacter::DashButtonReleased() 
{
	if(UPCharacterMovementComponent == nullptr) return;
	UPCharacterMovementComponent->DashReleased();
}

void AUntitledProjectCharacter::JumpButtonPressed() 
{
	if(UPCharacterMovementComponent->bWantsToCrouch)
	{
		UPCharacterMovementComponent->UnCrouch();
		UPCharacterMovementComponent->ExitSlide();
		bPressedShinedownJump = true;
		Jump();
	}
	Jump();
	bPressedShinedownJump = true;
	bIsJumping = true;
	CurrentJumpCount++;
	if(CurrentJumpCount == 2)
	{
		LaunchCharacter(FVector(0,0,UPCharacterMovementComponent->JumpZVelocity),false,true);
	}
}

void AUntitledProjectCharacter::EquipButtonPressed() 
{
	if(CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void AUntitledProjectCharacter::FireButtonPressed()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->FireButtonPressed(true);
	}
}

void AUntitledProjectCharacter::FireButtonReleased()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->FireButtonPressed(false);
	}
}

void AUntitledProjectCharacter::ReloadButtonPressed()
{
	if (CombatComponent && CombatComponent->EquippedWeapon && !CombatComponent->EquippedWeapon->IsFull())
	{
		UE_LOG(LogTemp, Warning, TEXT("Reload button pressed"));

		CombatComponent->Reload();
	}
}

void AUntitledProjectCharacter::MeleeButtonPressed()
{
	if (CombatComponent && !bMelee)
	{
		CombatComponent->PerformMelee();
		UAnimInstance* AnimInstance = GetMesh1P()->GetAnimInstance();
		if (AnimInstance && MeleeMontage)
		{
			AnimInstance->Montage_Play(MeleeMontage);
		UE_LOG(LogTemp, Warning, TEXT("Melee Animation Playing"));
		}
		bMelee = true;
		UE_LOG(LogTemp, Warning, TEXT("MeleeButtonPressed"));
	}
}

void AUntitledProjectCharacter::SwapWeaponsButtonPressed()
{
	if (CombatComponent && CombatComponent->ShouldSwapWeapons())
	{
		CombatComponent->SwapWeapons();
		CombatComponent->EquippedWeapon->SetHUDAmmo();
	}
}

void AUntitledProjectCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamgeType, AController* InstigatorController, AActor* DamageCauser)
{
	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	UpdateHUDHealth();
	UpdateHUDShield();
	if (Health <= 0)
	{
		if (bAI)
		{
			UE_LOG(LogTemp, Warning, TEXT("AIDead"));
			AShinedownPlayerState* InstigatorPlayerState = InstigatorController ? Cast<AShinedownPlayerState>(InstigatorController->PlayerState) : nullptr;
			if (InstigatorPlayerState)
			{
				UE_LOG(LogTemp, Warning, TEXT("SettingScoreForKilledAI"));
				InstigatorPlayerState->AddToScore(1.f);
			}
			if (bBoss)
			{
				AShinedownGameMode* GameMode = Cast<AShinedownGameMode>(GetWorld()->GetAuthGameMode());
				if (GameMode)
				{
					GameMode->EndMatch();
					AShinedownPlayerController* ShinedownInstigator = Cast<AShinedownPlayerController>(InstigatorController);
					bBossKilled = true;
					if (DeathMusic)
					{
						UGameplayStatics::PlaySound2D(this, DeathMusic);
					}
					if (ShinedownInstigator)
					{
						ShinedownInstigator->GameHasEnded(nullptr, bBossKilled);
					}

				}
			}
		}
		else
		{
			ShinedownPlayerState = ShinedownPlayerState == nullptr ? GetPlayerState<AShinedownPlayerState>() : ShinedownPlayerState;
			if (ShinedownPlayerState)
			{
				UE_LOG(LogTemp, Warning, TEXT("AddingToDefeats"));
				ShinedownPlayerState->AddToDefeats(1.f);
				AShinedownGameMode* GameMode =Cast<AShinedownGameMode>(GetWorld()->GetAuthGameMode());
				if (GameMode)
				{
					GameMode->EndMatch();
					ShinedownPlayerController->GameHasEnded();
					if (DeathMusic)
					{
						UGameplayStatics::PlaySound2D(this,DeathMusic);
					}
				}
			}
		}
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->StopMovementImmediately();
		DetachFromControllerPendingDestroy();
		CombatComponent->bCanFire = false;
		GetMesh()->SetVisibility(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Ignore);
		if (Damage > 100.f)
		{
			GetMesh()->AddImpulse(-GetActorForwardVector() * 10000.f, NAME_None, true);
		}
		GetMesh1P()->SetVisibility(false);
		
		CombatComponent->EquippedWeapon->Destroy();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);		
		bDead = true;
	}

}

void AUntitledProjectCharacter::UpdateHUDHealth()
{
	ShinedownPlayerController = ShinedownPlayerController == nullptr ? Cast<AShinedownPlayerController>(Controller) : ShinedownPlayerController;

	if (ShinedownPlayerController)
	{
		ShinedownPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AUntitledProjectCharacter::UpdateHUDShield()
{
	ShinedownPlayerController = ShinedownPlayerController == nullptr ? Cast<AShinedownPlayerController>(Controller) : ShinedownPlayerController;

	if (ShinedownPlayerController)
	{
		ShinedownPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void AUntitledProjectCharacter::UpdateHUDAmmo()
{
	ShinedownPlayerController = ShinedownPlayerController == nullptr ? Cast<AShinedownPlayerController>(Controller) : ShinedownPlayerController;

	if (ShinedownPlayerController)
	{
		ShinedownPlayerController->SetHUDCarriedAmmo(CombatComponent->CarriedAmmo);
		ShinedownPlayerController->SetHUDAmmo(CombatComponent->EquippedWeapon->GetAmmo());
	}
}

void AUntitledProjectCharacter::PollInit()
{
	ShinedownPlayerState = ShinedownPlayerState == nullptr ? GetPlayerState<AShinedownPlayerState>() : ShinedownPlayerState;
	if (ShinedownPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pollinit"));
		ShinedownPlayerState->AddToScore(0.f);
		ShinedownPlayerState->AddToDefeats(0);

	}
}

void AUntitledProjectCharacter::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (DefaultWeaponClass && Health != 0)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		if (CombatComponent)
		{
			CombatComponent->EquipPrimaryWeapon(StartingWeapon);
		}

	}
}

void AUntitledProjectCharacter::Landed(const FHitResult & Hit) 
{
	Super::Landed(Hit);

	bIsJumping = false;
	CurrentJumpCount = 0;
}


void AUntitledProjectCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AUntitledProjectCharacter::GetHasRifle()
{
	return bHasRifle;
}

void AUntitledProjectCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled() && HasAuthority())
	{
		if (OverlappingWeapon)
		{
		OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool AUntitledProjectCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

AWeapon* AUntitledProjectCharacter::GetEquippedWeapon()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr)
	{
		return nullptr;
	}
	return CombatComponent->EquippedWeapon;
}

ECombatState AUntitledProjectCharacter::GetCombatState() const
{
	if (CombatComponent == nullptr)
	{
		return ECombatState::ECS_MAX;
	}
	return CombatComponent->CombatState;
}
