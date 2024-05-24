// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "UntitledProjectCharacter.h"

UBuffComponent::UBuffComponent()
{

	PrimaryComponentTick.bCanEverTick = true;
	Character = Cast<AUntitledProjectCharacter>(GetOwner());
	
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	AmountToHeal += HealAmount;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealAmount, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Heal Buff"));

}

void UBuffComponent::Shield(float ShieldAmount)
{
	bShielding = true;
	AmountToShield += ShieldAmount;
	Character->SetShield(FMath::Clamp(Character->GetShield() + AmountToShield, 0.f, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	if (AmountToShield <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bShielding = false;
		AmountToShield = 0.f;
	}
}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

