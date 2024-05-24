// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Keycard.generated.h"

/**
 * 
 */
UCLASS()
class UNTITLEDPROJECT_API AKeycard : public APickup
{
	GENERATED_BODY()
public:
	virtual void Destroyed() override;
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

};
