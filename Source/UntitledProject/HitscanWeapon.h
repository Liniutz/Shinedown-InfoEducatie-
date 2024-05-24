// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitscanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class UNTITLEDPROJECT_API AHitscanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;
protected:
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

	UPROPERTY(EditAnywhere)
	class USoundCue* HitSound;
	UPROPERTY(EditAnywhere)
	USoundCue* CorpseHitSound;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

private:
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	//Trace end with scatter
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

};
