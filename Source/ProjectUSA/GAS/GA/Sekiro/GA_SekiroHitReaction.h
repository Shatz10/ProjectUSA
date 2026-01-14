// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroHitReaction.generated.h"

/**
 * Hit reaction ability for Sekiro-style damage responses.
 * 
 * Logic:
 * - Triggered by damage events (via GameplayEffect or direct activation)
 * - Selects appropriate reaction montage based on:
 *   - Damage level (light/medium/heavy/knockdown)
 *   - Character state (grounded/airborne)
 *   - Hit direction (front/back/left/right)
 * - Applies stun duration and appropriate tags
 * 
 * NOTE: This should be added to GameplayAbilities_Trigger and activated via damage events.
 */

UENUM(BlueprintType)
enum class EHitDamageLevel : uint8
{
	Light = 0,
	Medium = 1,
	Heavy = 2,
	VeryHeavy = 3,
	Knockdown = 4
};

UENUM(BlueprintType)
enum class EHitDirection : uint8
{
	Front = 0,
	Back = 1,
	Left = 2,
	Right = 3
};

UCLASS()
class PROJECTUSA_API UGA_SekiroHitReaction : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroHitReaction();

	/** Hit reaction montages for grounded state */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro|Ground")
	TMap<EHitDamageLevel, UAnimMontage*> GroundHitMontages;

	/** Hit reaction montages for airborne state */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro|Air")
	TMap<EHitDamageLevel, UAnimMontage*> AirHitMontages;

	/** Knockdown montage */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro|Knockdown")
	UAnimMontage* KnockdownMontage;

	/** Get up montage */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro|Knockdown")
	UAnimMontage* GetUpMontage;

	/** Tag applied during hit stun */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag HitStunTag;

	/** Tag applied during knockdown */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag KnockdownTag;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnHitReactionCompleted();
	UFUNCTION()
	void OnHitReactionCancelled();

	UAnimMontage* SelectHitMontage(EHitDamageLevel DamageLevel, bool bIsAirborne, EHitDirection Direction);
	EHitDamageLevel DetermineDamageLevel(float Damage, float MaxHealth);
	EHitDirection DetermineHitDirection(AActor* Attacker, AActor* Victim);

private:
	bool bIsKnockedDown;
};
