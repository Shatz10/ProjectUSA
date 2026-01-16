// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroResurrection.generated.h"

/**
 * Resurrection (起死回生) ability.
 * 
 * Logic:
 * - Triggered on "death" (Out of health).
 * - Costs 1.0 Resurrection Power.
 * - Restores 50% Health.
 * - Applies "State.NoResurrect" (Black Mark) tag.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroResurrection : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroResurrection();

	/** Health percentage to restore (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	float RestoreHealthPercentage = 0.5f;

	/** Tag applied after resurrecting to prevent consecutive resurrections */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag NoResurrectTag;

	/** Montage for the resurrection animation */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	UAnimMontage* ResurrectMontage;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	void PerformResurrection();
};
