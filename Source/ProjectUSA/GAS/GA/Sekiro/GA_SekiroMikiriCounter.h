// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroMikiriCounter.generated.h"

/**
 * Mikiri Counter ability.
 * 
 * Triggered when dashing into a perilous thrust attack.
 * Logic:
 * - Plays the Mikiri Stomp montage.
 * - Applies massive posture damage to the attacker.
 * - Briefly stuns the attacker.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroMikiriCounter : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroMikiriCounter();

	/** Montage for the stomp animation */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	UAnimMontage* MikiriMontage;

	/** Amount of posture damage to deal to the enemy */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	float PostureDamage = 40.f;

	/** Tag used to trigger this ability */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag MikiriEventTag;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	void ApplyMikiriImpact(AActor* Target);
};
