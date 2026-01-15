// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroCombatArt_Ichimonji.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/USAAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGA_SekiroCombatArt_Ichimonji::UGA_SekiroCombatArt_Ichimonji()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	SpiritEmblemCost = 0; // Ichimonji is free in Sekiro
	PostureRecoveryAmount = 20.f;
}

void UGA_SekiroCombatArt_Ichimonji::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 1. Setup hit listener
	UAbilityTask_WaitGameplayEvent* WaitHitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Combat.Hit")));
	WaitHitTask->EventReceived.AddDynamic(this, &UGA_SekiroCombatArt_Ichimonji::OnAttackHit);
	WaitHitTask->ReadyForActivation();

	// 2. Perform base combat art logic (montage, etc.)
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_SekiroCombatArt_Ichimonji::OnAttackHit(FGameplayEventData Payload)
{
	// Recover player posture
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		float CurrentPosture = ASC->GetNumericAttribute(UUSAAttributeSet::GetCurrentPostureAttribute());
		float NewPosture = FMath::Max(0.f, CurrentPosture - PostureRecoveryAmount);
		
		ASC->SetNumericAttributeBase(UUSAAttributeSet::GetCurrentPostureAttribute(), NewPosture);
		
		UE_LOG(LogTemp, Log, TEXT("Ichimonji Hit! Recovered %f posture. New Posture: %f"), PostureRecoveryAmount, NewPosture);
	}
}
