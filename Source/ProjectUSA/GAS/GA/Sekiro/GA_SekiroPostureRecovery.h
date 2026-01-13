// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroPostureRecovery.generated.h"

/**
 * Passive ability that ticks posture recovery over time.
 * 
 * Logic:
 * - Reduces CurrentPosture by PostureRecoverRate per second.
 * - Recovery rate can be modified by health percentage (lower health = slower recovery).
 * 
 * NOTE: This should be added to GameplayAbilities_Start in the SekiroHeroCharacter Blueprint.
 * It will activate automatically and run in the background.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroPostureRecovery : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroPostureRecovery();

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	float RecoveryTickInterval = 0.1f;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void RecoverPosture();

	FTimerHandle RecoveryTimerHandle;
};
