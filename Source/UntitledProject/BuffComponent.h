// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNTITLEDPROJECT_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuffComponent();
	friend class AUntitledProjectCharacter;
	void Heal(float HealAmount, float HealingTime);
	void Shield(float ShieldAmount);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	class AUntitledProjectCharacter* Character = nullptr;

	bool bHealing = false;
	float AmountToHeal = 0.f;
	float AmountToShield = 0.f;
	bool bShielding;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
