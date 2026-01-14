// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SekiroAttackWindow.generated.h"

/**
 * Animation notify to mark attack collision window frames.
 * 
 * Logic:
 * - Sends a GameplayEvent when triggered.
 * - The event can be caught by abilities to enable/disable weapon collision.
 * - Typically placed at the start and end of attack swing frames.
 * 
 * NOTE: Add this notify to attack animation montages in the Animation Editor.
 * Set EventTag to "GameplayEvent.Combat.AttackWindow.Start" or ".End"
 */
UCLASS()
class PROJECTUSA_API UAnimNotify_SekiroAttackWindow : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_SekiroAttackWindow();

	/** Tag to send as GameplayEvent */
	UPROPERTY(EditAnywhere, Category = "Sekiro")
	FGameplayTag EventTag;

	/** Whether this marks the start or end of attack window */
	UPROPERTY(EditAnywhere, Category = "Sekiro")
	bool bIsWindowStart = true;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
