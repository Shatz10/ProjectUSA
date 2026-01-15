// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroJumpCounter.generated.h"

/**
 * Jump Counter ability (Countering Sweeps).
 * 
 * Triggered by jumping while an enemy is performing a sweep.
 * Logic:
 * - Plays a stomp montage.
 * - Deals heavy posture damage to the target.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroJumpCounter : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroJumpCounter();

	/** Montage for jumping on enemy head */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	UAnimMontage* StompMontage;

	/** Amount of posture damage */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	float PostureDamage = 30.f;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
