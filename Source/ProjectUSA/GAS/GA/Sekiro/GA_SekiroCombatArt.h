// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroCombatArt.generated.h"

/**
 * Base class for Sekiro's "Combat Arts" (流派招式).
 * 
 * Logic:
 * - Costs Spirit Emblems to activate.
 * - Usually involves a specific montage sequence.
 * - Can have multiple stages/combos.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroCombatArt : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroCombatArt();

	/** Cost in Spirit Emblems */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	int32 SpiritEmblemCost = 2;

	/** Montage for the combat art */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	UAnimMontage* CombatArtMontage;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Consumes spirit emblems */
	void ConsumeResources();
};
