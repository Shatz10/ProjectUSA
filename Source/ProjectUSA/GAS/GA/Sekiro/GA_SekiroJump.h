// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_SekiroJump.generated.h"

/**
 * Sekiro-style directional jump ability.
 * 
 * Logic:
 * - Supports 8-directional jumps based on input
 * - State progression: Ready -> Start -> Loop -> End
 * - Uses montages for each state
 * - Applies appropriate tags during jump
 * 
 * NOTE: Add to GameplayAbilities_Active and bind to Jump button.
 */

UENUM(BlueprintType)
enum class EJumpDirection : uint8
{
	Forward = 0,
	ForwardLeft = 1,
	Left = 2,
	BackLeft = 3,
	Back = 4,
	BackRight = 5,
	Right = 6,
	ForwardRight = 7,
	InPlace = 8
};

UCLASS()
class PROJECTUSA_API UGA_SekiroJump : public UUSAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SekiroJump();

	/** Jump montages for different states */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro|Montages")
	UAnimMontage* JumpReadyMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro|Montages")
	UAnimMontage* JumpStartMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro|Montages")
	UAnimMontage* JumpLoopMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro|Montages")
	UAnimMontage* JumpEndMontage;

	/** Jump parameters */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro|Jump")
	float JumpHeight = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro|Jump")
	float JumpForwardSpeed = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Sekiro|Jump")
	float InPlaceJumpMultiplier = 0.2f;

	/** Tags */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	FGameplayTag JumpingTag;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnJumpStartCompleted();
	UFUNCTION()
	void OnJumpLoopCompleted();
	UFUNCTION()
	void OnJumpEndCompleted();
	UFUNCTION()
	void OnMontageInterrupted();

	EJumpDirection DetermineJumpDirection(const FVector& InputDirection, const FVector& ForwardVector);
	void ApplyJumpVelocity(ACharacter* Character, EJumpDirection Direction);

private:
	EJumpDirection CurrentJumpDirection;
};
