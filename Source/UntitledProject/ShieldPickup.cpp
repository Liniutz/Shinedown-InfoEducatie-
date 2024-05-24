// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldPickup.h"
#include "UntitledProjectCharacter.h"
#include "BuffComponent.h"

AShieldPickup::AShieldPickup()
{
}

void AShieldPickup::Destroyed()
{

	Super::Destroyed();
}

void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AUntitledProjectCharacter* Character = Cast<AUntitledProjectCharacter>(OtherActor);
	if (Character && !Character->IsAI())
	{

		UBuffComponent* Buff = Character->GetBuffComponnt();
		if (Buff)
		{
			Buff->Shield(ShieldAmount);
		}
		Destroy();
	}
}
