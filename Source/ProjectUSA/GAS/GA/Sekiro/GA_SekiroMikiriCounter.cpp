// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroMikiriCounter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/AttributeSet/USAAttributeSet.h"

UGA_SekiroMikiriCounter::UGA_SekiroMikiriCounter()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	PostureDamage = 40.f;
	
	// NOTE: In Blueprint, set MikiriEventTag = "GameplayEvent.Combat.Mikiri"
}

void UGA_SekiroMikiriCounter::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!TriggerEventData || !TriggerEventData->Target)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());
	
	// 1. Play Stomp Animation
	if (MikiriMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MikiriMontage);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SekiroMikiriCounter::K2_EndAbility);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SekiroMikiriCounter::K2_EndAbility);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SekiroMikiriCounter::K2_EndAbility);
		PlayMontageTask->ReadyForActivation();
	}

	// 2. Apply Posture Damage and Stun to Target
	ApplyMikiriImpact(TargetActor);

	if (!MikiriMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_SekiroMikiriCounter::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SekiroMikiriCounter::ApplyMikiriImpact(AActor* Target)
{
	if (!Target) return;

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target);
	if (!ASI) return;

	UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
	if (!TargetASC) return;

	// Apply Posture Damage via a GameplayEffect (or direct attribute modification for simplicity in C++)
	// In a real project, we'd use a GameplayEffect with an Execution Calculation.
	// For now, let's assume we can apply a simple modifier.
	
	// We'll send a Hit event to the attacker so they play their "Post-Mikiri" stumble animation
	FGameplayEventData Payload;
	Payload.Instigator = GetAvatarActorFromActorInfo();
	Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Combat.BeMikiriCountered"));
	TargetASC->HandleGameplayEvent(Payload.EventTag, &Payload);

	UE_LOG(LogTemp, Log, TEXT("Mikiri Counter applied to %s! Dealing %f posture damage."), *Target->GetName(), PostureDamage);
}
