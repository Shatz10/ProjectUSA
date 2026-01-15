// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroJumpCounter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_SekiroJumpCounter::UGA_SekiroJumpCounter()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	PostureDamage = 30.f;
}

void UGA_SekiroJumpCounter::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!TriggerEventData || !TriggerEventData->Target)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());

	// Play Stomp Animation
	if (StompMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, StompMontage);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SekiroJumpCounter::K2_EndAbility);
		PlayMontageTask->ReadyForActivation();
	}

	// Apply Posture Damage to Target
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetActor))
	{
		if (UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent())
		{
			// Send event to trigger stagger or damage
			FGameplayEventData Payload;
			Payload.Instigator = GetAvatarActorFromActorInfo();
			Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Combat.BeJumpCountered"));
			TargetASC->HandleGameplayEvent(Payload.EventTag, &Payload);
			
			UE_LOG(LogTemp, Log, TEXT("Jump Counter applied! Dealing heavy posture damage."));
		}
	}

	if (!StompMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}
