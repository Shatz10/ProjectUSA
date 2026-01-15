// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/USACharacterPlayer.h"
#include "SekiroHeroCharacter.generated.h"

/**
 * Sekiro-style character with Posture and Deflect mechanics.
 */
UCLASS()
class PROJECTUSA_API ASekiroHeroCharacter : public AUSACharacterPlayer
{
	GENERATED_BODY()
	
public:
	ASekiroHeroCharacter();

	virtual void SetupGAS() override;

	// Helper to get Sekiro-specific attributes
	float GetCurrentPosture() const;
	float GetMaxPosture() const;

	// Input buffer component for queuing inputs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro")
	class USekiroInputBufferComponent* InputBufferComponent;

protected:
	virtual void BeginPlay() override;

	// Handle Posture Break
	UFUNCTION()
	virtual void OnPostureBroken();

	// Input buffer integration - override from base class
	virtual void InputPressGameplayAbilityByInputID(int32 InputID);
	
	// Helper to try activating ability from buffered input
	void TryActivateAbilityWithBuffer(int32 InputID, FName InputName);
};
