// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SekiroParryWindow.generated.h"

/**
 * Animation notify to mark perfect parry timing window.
 * 
 * Logic:
 * - Adds/removes the "State.PerfectParry" tag when triggered.
 * - Used in deflect animations to mark the precise frames for perfect parry.
 * 
 * NOTE: Add this notify to deflect animation montages.
 * Place at the start of perfect parry frames, and use another at the end.
 */
UCLASS()
class PROJECTUSA_API UAnimNotify_SekiroParryWindow : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_SekiroParryWindow();

	/** Tag to add/remove */
	UPROPERTY(EditAnywhere, Category = "Sekiro")
	FGameplayTag ParryTag;

	/** Whether to add or remove the tag */
	UPROPERTY(EditAnywhere, Category = "Sekiro")
	bool bAddTag = true;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
