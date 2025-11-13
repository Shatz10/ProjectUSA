// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/USAGameplayAbility.h"
#include "GA_CheckCharacterCeiling.generated.h"

/**
 * 检查角色是否碰到天花板的能力
 * 
 * 使用场景: 用于需要检测天花板碰撞的能力
 */
UCLASS()
class PROJECTUSA_API UGA_CheckCharacterCeiling : public UUSAGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_CheckCharacterCeiling();

	/** Actually activate ability, do not call this directly */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;



	UPROPERTY(EditAnywhere, Category = "GAS")
	TArray <TSubclassOf<class UGameplayEffect>> InCeilingEffects;

	UPROPERTY(EditAnywhere, Category = "GAS")
	TArray < TSubclassOf<class UGameplayEffect>> OutCeilingEffects;

	UFUNCTION()
	void ApplayInCeilingEffects ();
	
	UFUNCTION()
	void ApplayOutCeilingEffects();


};
