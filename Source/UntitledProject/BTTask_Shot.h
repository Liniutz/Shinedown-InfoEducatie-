// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Shot.generated.h"

/**
 * 
 */
UCLASS()
class UNTITLEDPROJECT_API UBTTask_Shot : public UBTTaskNode
{
	GENERATED_BODY()
public:
	friend class AUntitledProjectCharacter;
	UBTTask_Shot();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
