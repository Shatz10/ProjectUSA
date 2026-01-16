// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA/Sekiro/GA_SekiroProstheticTool.h"
#include "GA_SekiroProsthetic_Shuriken.generated.h"

/**
 * Shuriken (手里剑) prosthetic tool ability.
 */
UCLASS()
class PROJECTUSA_API UGA_SekiroProsthetic_Shuriken : public UGA_SekiroProstheticTool
{
	GENERATED_BODY()

public:
	UGA_SekiroProsthetic_Shuriken();

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Sekiro")
	TSubclassOf<class ASekiroShurikenProjectile> ProjectileClass;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Spawns the projectile (can be called from an AnimNotify or timer) */
	void SpawnShuriken();
};
