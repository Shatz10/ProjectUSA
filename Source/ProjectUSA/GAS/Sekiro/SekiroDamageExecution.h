// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "SekiroDamageExecution.generated.h"

/**
 * Custom Execution Calculation for Sekiro-style damage.
 * 
 * Logic:
 * - If target has "State.Blocking" tag: Redirect damage to Posture instead of Health.
 * - If target has "State.PerfectParry" tag: Nullify damage and apply posture damage to attacker.
 * - Otherwise: Apply normal health damage.
 * 
 * NOTE: This should be used in a GameplayEffect Blueprint.
 * Create a GE_SekiroDamage Blueprint, set Execution Class to this, and configure modifiers.
 */
UCLASS()
class PROJECTUSA_API USekiroDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	USekiroDamageExecution();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
