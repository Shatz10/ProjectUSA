// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroAttack.generated.h"

/**
 * Sekiro-style Attack Ability.
 * Logic:
 * 1. Play Attack Montage.
 * 2. Handle Combo Logic via Notifies/Events.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroAttack : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroAttack();

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	UAnimMontage* AttackMontage;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnCompleted();
	UFUNCTION()
	void OnBlendOut();
	UFUNCTION()
	void OnInterrupted();
	UFUNCTION()
	void OnCancelled();
};
