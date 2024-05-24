// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "UntitledProjectCharacter.h"
#include "BuffComponent.h"

AHealthPickup::AHealthPickup()
{
}

void AHealthPickup::Destroyed()
{


	Super::Destroyed();
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	AUntitledProjectCharacter* Character = Cast<AUntitledProjectCharacter>(OtherActor);
	if (Character && !Character->IsAI())
	{
		UE_LOG(LogTemp, Warning, TEXT("BuffSet"));

		UBuffComponent* Buff = Character->GetBuffComponnt();
		if (Buff)
		{
			UE_LOG(LogTemp, Warning, TEXT("HealingStart"));

			Buff->Heal(HealAmount, HealingTime);
		}
		Destroy();
	}
}
