// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroCrouch.generated.h"

/**
 * Couching ability for stealth.
 * 
 * Logic:
 * - Toggles the State.Crouching tag.
 * - Reduces movement speed.
 * - AI integration (via tag) to reduce detection.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroCrouch : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroCrouch();

	/** Speed multiplier during crouch */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	float CrouchSpeedMultiplier = 0.5f;

	/** Tag for crouching */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag CrouchingTag;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	float OriginalMaxWalkSpeed;
};
