// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroDeflect.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/AttributeSet/USAAttributeSet.h"

UGA_SekiroDeflect::UGA_SekiroDeflect()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	PerfectParryWindow = 0.2f;

	// NOTE: In Blueprint, set these tags:
	// BlockingTag = "State.Blocking"
	// PerfectParryTag = "State.PerfectParry"
	// HitEventTag = "GameplayEvent.Combat.Hit"
}

void UGA_SekiroDeflect::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Apply Blocking tag
	if (BlockingTag.IsValid() && GetAbilitySystemComponentFromActorInfo())
	{
		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(BlockingTag);
	}

	// Apply Perfect Parry tag for the window duration
	if (PerfectParryTag.IsValid() && GetAbilitySystemComponentFromActorInfo())
	{
		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(PerfectParryTag);
		
		// Remove perfect parry tag after window expires
		FTimerHandle ParryWindowTimer;
		GetWorld()->GetTimerManager().SetTimer(ParryWindowTimer, [this]()
		{
			if (GetAbilitySystemComponentFromActorInfo())
			{
				GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(PerfectParryTag);
			}
		}, PerfectParryWindow, false);
	}

	// Play Block Animation
	if (BlockMontage)
	{
		// Ideally use AT_PlayAnimMontages or standard task
		// For simplicity using standard:
		// UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, BlockMontage);
		// PlayMontageTask->ReadyForActivation();
		// In Sekiro, blocking is a loop/pose, usually handled by AnimBP via Tag or specific Montage Loop.
		// We'll assume applying a Tag 'State.Blocking' drives the AnimBP for now.
	}

	// Wait for Input Release
	UAbilityTask_WaitInputRelease* WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_SekiroDeflect::OnReleaseInput);
	WaitInputReleaseTask->ReadyForActivation();

	// Listen for Hits (for Parry)
	if (HitEventTag.IsValid())
	{
		UAbilityTask_WaitGameplayEvent* WaitHitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, HitEventTag, nullptr, false, false);
		WaitHitTask->EventReceived.AddDynamic(this, &UGA_SekiroDeflect::OnHitReceived);
		WaitHitTask->ReadyForActivation();
	}
	
	StartTime = GetWorld()->GetTimeSeconds();
}

void UGA_SekiroDeflect::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Remove tags
	if (GetAbilitySystemComponentFromActorInfo())
	{
		if (BlockingTag.IsValid())
		{
			GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(BlockingTag);
		}
		if (PerfectParryTag.IsValid())
		{
			GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(PerfectParryTag);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SekiroDeflect::OnReleaseInput(float TimeWaited)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SekiroDeflect::OnHitReceived(FGameplayEventData Payload)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - StartTime <= PerfectParryWindow)
	{
		UE_LOG(LogTemp, Log, TEXT("Perfect Parry!"));
		// Logic: Apply 0 damage, Deal posture damage to attacker
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Normal Block"));
		// Logic: Reduce damage, Deal posture damage to self
	}
}
