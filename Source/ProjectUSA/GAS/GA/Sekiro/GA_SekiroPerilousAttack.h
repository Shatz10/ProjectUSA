// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroPerilousAttack.generated.h"

/**
 * Types of perilous attacks in Sekiro.
 */
UENUM(BlueprintType)
enum class ESekiroPerilousType : uint8
{
	Thrust,
	Sweep,
	Grab
};

/**
 * Base class for Sekiro's "Perilous Attacks" (the ones that show the "危" sign).
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroPerilousAttack : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroPerilousAttack();

	/** Type of this perilous attack */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	ESekiroPerilousType PerilousType;

	/** Tag applied to character during the "Danger" warning window */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag DangerTag;

	/** Tag that identifies this attack as perilous to the damage execution */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag PerilousAttackTag;

	/** Montage to play */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	UAnimMontage* AttackMontage;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** Sends the UI event to show the "Danger" kanji */
	void NotifyDangerUI();
};
