// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/USACharacterPlayer.h"
#include "Perception/AISightTargetInterface.h"
#include "SekiroHeroCharacter.generated.h"

/**
 * Sekiro-style character with Posture and Deflect mechanics.
 */
UCLASS()
class PROJECTUSA_API ASekiroHeroCharacter : public AUSACharacterPlayer, public IAISightTargetInterface
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

	/** Target Lock-on component (usually added in BP) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro")
	class USekiroTargetLockComponent* TargetLockComponent;

	// Helper to get Spirit Emblems
	float GetCurrentSpiritEmblems() const;
	float GetMaxSpiritEmblems() const;

	// Cycle through equipped prosthetic tools
	UFUNCTION(BlueprintCallable, Category = "Sekiro")
	void CycleProstheticTool();

	// Get currently selected prosthetic ability class
	UFUNCTION(BlueprintPure, Category = "Sekiro")
	TSubclassOf<class UGA_SekiroProstheticTool> GetCurrentProstheticTool() const;

	/** List of equipped prosthetic abilities */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro")
	TArray<TSubclassOf<class UGA_SekiroProstheticTool>> EquippedTools;

	/** Index of the currently selected tool */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro")
	int32 CurrentToolIndex = 0;

	/** Visibility modifier based on current state (Crouching, etc.) */
	UFUNCTION(BlueprintPure, Category = "Sekiro")
	float GetVisibilityMultiplier() const;

protected:
	virtual void BeginPlay() override;

	// Handle Posture Break
	UFUNCTION()
	virtual void OnPostureBroken();

	// Input buffer integration - override from base class
	virtual void InputPressGameplayAbilityByInputID(int32 InputID);
	
	// Helper to try activating ability from buffered input
	void TryActivateAbilityWithBuffer(int32 InputID, FName InputName);

	// IAISightTargetInterface implementation
	virtual bool CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor = nullptr, const bool* bWasVisible = nullptr, int32* UserData = nullptr) const override;
};
