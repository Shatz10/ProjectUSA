// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroPostureRecovery.h"
#include "GAS/AttributeSet/USAAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"

UGA_SekiroPostureRecovery::UGA_SekiroPostureRecovery()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	RecoveryTickInterval = 0.1f;
}

void UGA_SekiroPostureRecovery::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Start recovery timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(RecoveryTimerHandle, this, &UGA_SekiroPostureRecovery::RecoverPosture, RecoveryTickInterval, true);
	}
}

void UGA_SekiroPostureRecovery::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SekiroPostureRecovery::RecoverPosture()
{
	if (!GetAbilitySystemComponentFromActorInfo())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const UUSAAttributeSet* AttributeSet = ASC->GetSet<UUSAAttributeSet>();

	if (!AttributeSet)
	{
		return;
	}

	float CurrentPosture = AttributeSet->GetCurrentPosture();
	float RecoverRate = AttributeSet->GetPostureRecoverRate();

	// Scale recovery by health percentage (optional)
	float HealthPercent = AttributeSet->GetCurrentHealth() / FMath::Max(AttributeSet->GetMaxHealth(), 1.f);
	float ScaledRecovery = RecoverRate * RecoveryTickInterval * HealthPercent;

	// Apply recovery (reduce posture)
	float NewPosture = FMath::Max(CurrentPosture - ScaledRecovery, 0.f);

	// Apply via GameplayEffect would be better, but for simplicity:
	// NOTE: In production, create a GE_PostureRecovery and apply it here
	ASC->SetNumericAttributeBase(AttributeSet->GetCurrentPostureAttribute(), NewPosture);
}
