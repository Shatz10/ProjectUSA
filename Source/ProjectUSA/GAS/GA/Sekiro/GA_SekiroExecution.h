// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroExecution.generated.h"

/**
 * Sekiro-style Execution/Deathblow ability.
 * 
 * Logic:
 * - Can only be activated when target has "State.PostureBroken" tag.
 * - Plays execution animation.
 * - Deals massive damage or instant kill.
 * 
 * NOTE: This should be bound to an input action in Blueprint.
 * The ability will check if a valid target is in range and has broken posture.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroExecution : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroExecution();

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	UAnimMontage* ExecutionMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	float ExecutionRange = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	float ExecutionDamage = 9999.f;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const override;

	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageCancelled();

	virtual AActor* FindExecutionTarget();
};
