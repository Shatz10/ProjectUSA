// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/SekiroTargetLockComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

USekiroTargetLockComponent::USekiroTargetLockComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	LockedTarget = nullptr;
}

void USekiroTargetLockComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USekiroTargetLockComponent::ToggleLockOn()
{
	if (LockedTarget)
	{
		LockedTarget = nullptr;
		UE_LOG(LogTemp, Log, TEXT("Target Lock Off"));
	}
	else
	{
		LockedTarget = FindBestTarget();
		if (LockedTarget)
		{
			UE_LOG(LogTemp, Log, TEXT("Target Locked: %s"), *LockedTarget->GetName());
		}
	}
}

AActor* USekiroTargetLockComponent::FindBestTarget()
{
	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	TArray<AActor*> OverlappingActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Owner);

	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Owner->GetActorLocation(),
		LockRange,
		ObjectTypes,
		nullptr,
		IgnoreActors,
		OverlappingActors
	);

	if (!bHit) return nullptr;

	AActor* BestTarget = nullptr;
	float BestDot = -1.0f;

	FVector Forward = Owner->GetActorForwardVector();

	for (AActor* Actor : OverlappingActors)
	{
		FVector ToActor = (Actor->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
		float Dot = FVector::DotProduct(Forward, ToActor);

		if (Dot > BestDot && Dot > 0.5f) // Must be in front (45 degrees)
		{
			BestDot = Dot;
			BestTarget = Actor;
		}
	}

	return BestTarget;
}
