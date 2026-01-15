// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Sekiro/GA_SekiroExecution.h"
#include "GA_SekiroStealthKill.generated.h"

/**
 * Stealth Kill execution ability.
 * 
 * Triggered when behind an unaware enemy or from above.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroStealthKill : public UGA_SekiroExecution
{
	GENERATED_BODY()

public:
	UGA_SekiroStealthKill();

	/** Tag that identifies an unaware enemy */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag UnawareTag;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// Overriding to check for stealth status instead of posture break
	virtual AActor* FindExecutionTarget() override;
};
