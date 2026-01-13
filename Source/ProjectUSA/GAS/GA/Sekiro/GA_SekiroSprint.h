// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroSprint.generated.h"

/**
 * Sekiro-style Sprint toggle ability.
 * 
 * Logic:
 * - Toggles sprint mode on/off.
 * - Increases movement speed while active.
 * - Can be interrupted by other actions.
 * 
 * NOTE: Add to GameplayAbilities_Active in Blueprint and bind to Sprint button.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroSprint : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroSprint();

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	float SprintSpeedMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag SprintingTag;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

private:
	float OriginalMaxWalkSpeed;
};
