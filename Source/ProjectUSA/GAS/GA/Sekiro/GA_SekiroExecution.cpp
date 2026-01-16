// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroExecution.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UGA_SekiroExecution::UGA_SekiroExecution()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ExecutionRange = 200.f;
	ExecutionDamage = 9999.f;
}

bool UGA_SekiroExecution::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check if there's a valid target with broken posture
	// NOTE: This is a simplified check. In production, you'd cache the target.
	return true;
}

void UGA_SekiroExecution::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* Target = FindExecutionTarget();
	if (!Target)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. Clear Resurrection Black Mark (State.NoResurrect)
	// In Sekiro, performing a deathblow clears the black mark allowing for another resurrection.
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.NoResurrect")));
		UE_LOG(LogTemp, Log, TEXT("Execution performed: Resurrection Black Mark cleared."));
	}

	// 2. Play execution montage
	if (ExecutionMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ExecutionMontage);
		
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SekiroExecution::OnMontageCompleted);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SekiroExecution::OnMontageCancelled);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SekiroExecution::OnMontageCancelled);

		PlayMontageTask->ReadyForActivation();
	}

	// 3. Apply execution damage
	UGameplayStatics::ApplyDamage(Target, ExecutionDamage, GetOwningActorFromActorInfo()->GetInstigatorController(), GetOwningActorFromActorInfo(), UDamageType::StaticClass());
}

void UGA_SekiroExecution::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SekiroExecution::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

AActor* UGA_SekiroExecution::FindExecutionTarget()
{
	AActor* OwnerActor = GetOwningActorFromActorInfo();
	if (!OwnerActor)
	{
		return nullptr;
	}

	// Simple sphere trace to find target
	TArray<FHitResult> HitResults;
	FVector StartLocation = OwnerActor->GetActorLocation();
	FVector EndLocation = StartLocation + OwnerActor->GetActorForwardVector() * ExecutionRange;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);

	GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(100.f), QueryParams);

	for (const FHitResult& Hit : HitResults)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			// Check if target has PostureBroken tag
			// NOTE: You need to implement IAbilitySystemInterface on your characters
			// For now, just return the first valid target
			return HitActor;
		}
	}

	return nullptr;
}
