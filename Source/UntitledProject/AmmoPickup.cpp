// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "UntitledProjectCharacter.h"
#include "CombatComponent.h"


void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);


	AUntitledProjectCharacter* ShinedownCharacter = Cast<AUntitledProjectCharacter>(OtherActor);
	if (ShinedownCharacter && !ShinedownCharacter->IsAI())
	{
		UCombatComponent* Combat = ShinedownCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
		Destroy();
	}
}
