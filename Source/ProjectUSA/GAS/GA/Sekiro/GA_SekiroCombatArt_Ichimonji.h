// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Sekiro/GA_SekiroCombatArt.h"
#include "GA_SekiroCombatArt_Ichimonji.generated.h"

/**
 * Ichimonji combat art.
 * 
 * Logic:
 * - Deals high posture damage.
 * - Recovers a portion of the player's posture on successful hit.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroCombatArt_Ichimonji : public UGA_SekiroCombatArt
{
	GENERATED_BODY()

public:
	UGA_SekiroCombatArt_Ichimonji();

	/** Amount of posture to recover on hit */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	float PostureRecoveryAmount = 20.f;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Called when the attack hits an enemy */
	UFUNCTION()
	void OnAttackHit(FGameplayEventData Payload);
};
