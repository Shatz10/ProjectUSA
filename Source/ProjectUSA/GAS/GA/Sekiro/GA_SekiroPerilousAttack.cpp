// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroPerilousAttack.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_SekiroPerilousAttack::UGA_SekiroPerilousAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	PerilousType = ESekiroPerilousType::Thrust;
}

void UGA_SekiroPerilousAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. Notify UI to show "Danger" Kanji
	NotifyDangerUI();

	// 2. Apply tags if necessary (usually handled via AbilityTags in CDO, but can be manual)
	if (PerilousAttackTag.IsValid())
	{
		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(PerilousAttackTag);
	}

	// 3. Play Montage
	if (AttackMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SekiroPerilousAttack::K2_EndAbility);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SekiroPerilousAttack::K2_EndAbility);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SekiroPerilousAttack::K2_EndAbility);
		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_SekiroPerilousAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (PerilousAttackTag.IsValid())
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(PerilousAttackTag);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SekiroPerilousAttack::NotifyDangerUI()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayEventData Payload;
		Payload.Instigator = GetAvatarActorFromActorInfo();
		Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("GameplayEvent.UI.DangerNotice"));
		
		// Send event to self (UI will listen to this on the local player)
		ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
}
