// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"
#include "Components/SphereComponent.h"
#include "UntitledProjectCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ADoor::ADoor()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	SetRootComponent(DoorMesh);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(RootComponent);
	Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void ADoor::BeginPlay()
{
	Super::BeginPlay();
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
}

void ADoor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AUntitledProjectCharacter* ShinedownCharacter = Cast<AUntitledProjectCharacter>(OtherActor);

	if (ShinedownCharacter && !ShinedownCharacter->IsAI())
	{
		ShinedownCharacter->SetOverlappingDoor(true);
		if (ShinedownCharacter->HasKeycard())
		{
			FVector NewLocation = GetActorLocation();
			NewLocation.Z += 800.f;
			SetActorLocation(NewLocation);
			if (OpenSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, OpenSound, GetActorLocation());
			}
		}
	}
}

void ADoor::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AUntitledProjectCharacter* ShinedownCharacter = Cast<AUntitledProjectCharacter>(OtherActor);

	if (ShinedownCharacter && !ShinedownCharacter->IsAI())
	{
		ShinedownCharacter->SetOverlappingDoor(false);
	}
}


void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

