// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Shot.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ShinedownAIController.h"
#include "UntitledProjectCharacter.h"
#include "CombatComponent.h"
#include "Weapon.h"

UBTTask_Shot::UBTTask_Shot()
{
	NodeName = TEXT("Shot");
}

EBTNodeResult::Type UBTTask_Shot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	AUntitledProjectCharacter* ShinedownAICharacter = Cast<AUntitledProjectCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (ShinedownAICharacter == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	UCombatComponent* CombatComp = ShinedownAICharacter->GetCombat();
	ShinedownAICharacter->AIShoot();
	if (CombatComp && ShinedownAICharacter->GetEquippedWeapon()->IsEmpty())
	{
		CombatComp->Reload();
	}
	return EBTNodeResult::Succeeded;

}