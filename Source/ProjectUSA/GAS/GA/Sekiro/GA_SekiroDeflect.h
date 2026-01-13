// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroDeflect.generated.h"

/**
 * Sekiro-style Deflect/Block Ability.
 * Logic:
 * 1. Play Start Animation (Block Pose).
 * 2. Add 'State.Blocking' Tag.
 * 3. Listen for Hit Events for Perfect Parry window.
 * 4. Wait for Input Release to Stop.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroDeflect : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroDeflect();

	/** Time window for Perfect Parry in seconds */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	float PerfectParryWindow = 0.2f;

	/** Montage to play for Blocking */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	UAnimMontage* BlockMontage;

	/** Tag to identifying a Hit event */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag HitEventTag;

	/** Tag applied while blocking */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag BlockingTag;

	/** Tag applied during perfect parry window */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag PerfectParryTag;

	float StartTime;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnReleaseInput(float TimeWaited);

	UFUNCTION()
	void OnHitReceived(FGameplayEventData Payload);
};
