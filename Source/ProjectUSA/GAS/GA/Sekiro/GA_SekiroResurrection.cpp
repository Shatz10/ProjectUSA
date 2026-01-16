// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroResurrection.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/USAAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"

UGA_SekiroResurrection::UGA_SekiroResurrection()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	RestoreHealthPercentage = 0.5f;
	
	// NOTE: In Blueprint, set NoResurrectTag = "State.NoResurrect"
}

bool UGA_SekiroResurrection::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC) return false;

	// 1. Check if we have enough power (at least 1.0)
	float Power = ASC->GetNumericAttribute(UUSAAttributeSet::GetResurrectionPowerAttribute());
	if (Power < 1.0f) return false;

	// 2. Check if we have the "Black Mark" (NoResurrect tag)
	if (ASC->HasMatchingGameplayTag(NoResurrectTag))
	{
		return false;
	}

	return true;
}

void UGA_SekiroResurrection::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// In Sekiro, we usually wait for player input during the death screen.
	// For this prototype, we'll perform it immediately or after a short delay.
	
	PerformResurrection();

	if (ResurrectMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ResurrectMontage);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SekiroResurrection::K2_EndAbility);
		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_SekiroResurrection::PerformResurrection()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	// 1. Consume 1.0 Power
	float Power = ASC->GetNumericAttribute(UUSAAttributeSet::GetResurrectionPowerAttribute());
	ASC->SetNumericAttributeBase(UUSAAttributeSet::GetResurrectionPowerAttribute(), FMath::Max(0.f, Power - 1.0f));

	// 2. Restore Health
	float MaxHealth = ASC->GetNumericAttribute(UUSAAttributeSet::GetMaxHealthAttribute());
	ASC->SetNumericAttributeBase(UUSAAttributeSet::GetCurrentHealthAttribute(), MaxHealth * RestoreHealthPercentage);

	// 3. Apply Black Mark tag
	if (NoResurrectTag.IsValid())
	{
		ASC->AddLooseGameplayTag(NoResurrectTag);
	}

	UE_LOG(LogTemp, Warning, TEXT("Shinobi Resurrected!"));
}
