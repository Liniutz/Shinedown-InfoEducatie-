// Fill out your copyright notice in the Description page of Project Settings.


#include "UPPlayerCameraManager.h"
#include "UPCharacterMovementComponent.h"
#include "UntitledProjectCharacter.h"
#include "Components/CapsuleComponent.h"

AUPPlayerCameraManager::AUPPlayerCameraManager() 
{
}

void AUPPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) 
{
    Super::UpdateViewTarget(OutVT, DeltaTime);

    if(AUntitledProjectCharacter* UPCharacter = Cast<AUntitledProjectCharacter>(GetOwningPlayerController()->GetPawn()))
    {
        UUPCharacterMovementComponent* UPCMC = UPCharacter->GetUPCMC();
        FVector TargetCrouchOffset = FVector(0,0,UPCMC->GetCrouchedHalfHeight() - UPCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
        FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffset, FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.f, 1.f));
        OriginalRotation = OutVT.POV.Rotation;
        if(UPCMC->IsCrouching())
        {
            CrouchBlendTime = FMath::Clamp(CrouchBlendTime + DeltaTime, 0.f, CrouchBlendDuration);
            Offset -= TargetCrouchOffset;
        }
        else
        {
            CrouchBlendTime = FMath::Clamp(CrouchBlendTime - DeltaTime, 0.f, CrouchBlendDuration);
        }
        FRotator Rotation = OriginalRotation;
        if(UPCMC->IsWallRunning())
        {
            FRotator TargetRotation (0.f,0.f,0.f);
            if(UPCMC->WallRunningIsRight())
            {
                TargetRotation.Roll = FMath::FInterpTo(TargetRotation.Roll, -30, DeltaTime, 10.f);
            }
            else if(!UPCMC->WallRunningIsRight())
            {
                TargetRotation.Roll = FMath::FInterpTo(TargetRotation.Roll, 30, DeltaTime, 10.f);
            }
            OutVT.POV.Rotation.Roll = TargetRotation.Roll;
        }
        else
        {
            OutVT.POV.Rotation.Roll = FMath::FInterpTo(OutVT.POV.Rotation.Roll, 0, DeltaTime, 10.f);
        }
        OutVT.POV.Location += Offset;

    }
}
