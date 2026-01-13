// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroDash.generated.h"

/**
 * Sekiro-style Dash ability.
 * 
 * Logic:
 * - Quick dash in input direction (or forward if no input).
 * - Provides brief invincibility frames via State.Dashing tag.
 * - Uses character launch for movement.
 * 
 * NOTE: Add to GameplayAbilities_Active in Blueprint and bind to Shift/Dodge button.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroDash : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroDash();

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	UAnimMontage* DashMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	float DashDistance = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	float DashDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag DashingTag;

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	bool bInvincibleDuringDash = true;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnDashCompleted();
};
