// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroProstheticTool.generated.h"

/**
 * Base class for Shinobi Prosthetics (忍义手).
 * 
 * Features:
 * - Costs Spirit Emblems.
 * - Can be upgraded (optional for now).
 * - Plays specific tool animations/effects.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroProstheticTool : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroProstheticTool();

	/** Cost in Spirit Emblems */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	int32 SpiritEmblemCost = 1;

	/** Montage for using the tool */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	UAnimMontage* ToolMontage;

	/** Tag that identifies this specific tool (e.g. Ability.Prosthetic.Shuriken) */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag ToolTag;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Consumes spirit emblems */
	void ConsumeResources();
};
