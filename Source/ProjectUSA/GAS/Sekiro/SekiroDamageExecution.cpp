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
	bool bIsBlocking = TargetASC && TargetASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Blocking")));
	bool bIsPerfectParry = TargetASC && TargetASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.PerfectParry")));

	// Check if source is performing a perilous attack
	bool bIsThrust = SourceASC && SourceASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Attack.Perilous.Thrust")));
	bool bIsSweep = SourceASC && SourceASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Attack.Perilous.Sweep")));
	bool bIsGrab = SourceASC && SourceASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Attack.Perilous.Grab")));
	bool bIsPerilous = bIsThrust || bIsSweep || bIsGrab;

	if (bIsPerfectParry)
	{
		// Grabs and Sweeps cannot be perfect parried (usually)
		if (bIsGrab || bIsSweep)
		{
			// Apply damage even if parrying
			OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, Damage));
			UE_LOG(LogTemp, Log, TEXT("Perilous Sweep/Grab hit through parry!"));
		}
		else
		{
			// Perfect Parry: Nullify damage (works for normal attacks and thrusts)
			UE_LOG(LogTemp, Log, TEXT("Perfect Parry! Damage nullified."));
		}
		return;
	}
	else if (bIsBlocking)
	{
		// Normal blocks don't work against perilous attacks
		if (bIsPerilous)
		{
			OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, Damage));
			UE_LOG(LogTemp, Log, TEXT("Perilous attack broke through block!"));
		}
		else
		{
			// Normal Block: Redirect damage to Posture
			OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().CurrentPostureProperty, EGameplayModOp::Additive, Damage));
			UE_LOG(LogTemp, Log, TEXT("Blocked! %f damage to Posture."), Damage);
		}
	}
	else
	{
		// Normal Hit: Apply to Health
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, Damage));
		UE_LOG(LogTemp, Log, TEXT("Hit! %f damage to Health."), Damage);
	}
}
