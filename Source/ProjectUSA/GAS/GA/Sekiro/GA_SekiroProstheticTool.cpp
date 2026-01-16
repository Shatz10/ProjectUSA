// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroProstheticTool.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GAS/AttributeSet/USAAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_SekiroProstheticTool::UGA_SekiroProstheticTool()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	SpiritEmblemCost = 1;
}

bool UGA_SekiroProstheticTool::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check for Spirit Emblems
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(ActorInfo->AvatarActor.Get()))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			float CurrentEmblems = ASC->GetNumericAttribute(UUSAAttributeSet::GetCurrentSpiritEmblemsAttribute());
			if (CurrentEmblems < SpiritEmblemCost)
			{
				UE_LOG(LogTemp, Warning, TEXT("Not enough Spirit Emblems to use Prosthetic Tool!"));
				return false;
			}
		}
	}

	return true;
}

void UGA_SekiroProstheticTool::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. Consume resources
	ConsumeResources();

	// 2. Play Montage
	if (ToolMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ToolMontage);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SekiroProstheticTool::K2_EndAbility);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SekiroProstheticTool::K2_EndAbility);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SekiroProstheticTool::K2_EndAbility);
		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_SekiroProstheticTool::ConsumeResources()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		float CurrentEmblems = ASC->GetNumericAttribute(UUSAAttributeSet::GetCurrentSpiritEmblemsAttribute());
		ASC->SetNumericAttributeBase(UUSAAttributeSet::GetCurrentSpiritEmblemsAttribute(), FMath::Max(0.f, CurrentEmblems - SpiritEmblemCost));
		
		UE_LOG(LogTemp, Log, TEXT("Using Prosthetic: Consumed %d Spirit Emblems. Remaining: %f"), SpiritEmblemCost, ASC->GetNumericAttribute(UUSAAttributeSet::GetCurrentSpiritEmblemsAttribute()));
	}
}
