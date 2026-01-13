// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Sekiro/SekiroDamageExecution.h"
#include "GAS/AttributeSet/USAAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Tag/USAGameplayTags.h"

// Declare attributes we'll capture
struct FSekiroDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CurrentHealth);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CurrentPosture);

	FSekiroDamageStatics()
	{
		// Capture Damage from Source (Attacker)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UUSAAttributeSet, Damage, Source, false);
		
		// Capture Health and Posture from Target (Defender)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UUSAAttributeSet, CurrentHealth, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UUSAAttributeSet, CurrentPosture, Target, false);
	}
};

static const FSekiroDamageStatics& DamageStatics()
{
	static FSekiroDamageStatics DmgStatics;
	return DmgStatics;
}

USekiroDamageExecution::USekiroDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().DamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().CurrentHealthDef);
	RelevantAttributesToCapture.Add(DamageStatics().CurrentPostureDef);
}

void USekiroDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// Gather tags
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Get base damage
	float Damage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageDef, EvaluationParameters, Damage);

	if (Damage <= 0.f)
	{
		return;
	}

	// Check if target is blocking
	// NOTE: You need to define these tags in USAGameplayTags.h
	// For now using placeholder logic
	bool bIsBlocking = TargetASC && TargetASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Blocking")));
	bool bIsPerfectParry = TargetASC && TargetASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.PerfectParry")));

	if (bIsPerfectParry)
	{
		// Perfect Parry: Nullify damage, apply posture damage to attacker
		// NOTE: In Blueprint, you would apply a separate GE to the Source here
		// For now, just nullify damage
		UE_LOG(LogTemp, Log, TEXT("Perfect Parry! Damage nullified."));
		return;
	}
	else if (bIsBlocking)
	{
		// Normal Block: Redirect damage to Posture
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().CurrentPostureProperty, EGameplayModOp::Additive, Damage));
		UE_LOG(LogTemp, Log, TEXT("Blocked! %f damage to Posture."), Damage);
	}
	else
	{
		// Normal Hit: Apply to Health
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, Damage));
		UE_LOG(LogTemp, Log, TEXT("Hit! %f damage to Health."), Damage);
	}
}
