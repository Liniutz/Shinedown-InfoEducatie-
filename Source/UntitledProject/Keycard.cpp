// Fill out your copyright notice in the Description page of Project Settings.


#include "Keycard.h"
#include "UntitledProjectCharacter.h"



void AKeycard::Destroyed()
{
	Super::Destroyed();
}

void AKeycard::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AUntitledProjectCharacter* Character = Cast<AUntitledProjectCharacter>(OtherActor);
	if (Character && !Character->IsAI())
	{
		Character->SetKeycard(true);

		Destroy();
	}
}
