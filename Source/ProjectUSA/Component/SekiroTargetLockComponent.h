// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SekiroTargetLockComponent.generated.h"

/**
 * Component to handle target locking logic, common in Sekiro-style combat.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTUSA_API USekiroTargetLockComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USekiroTargetLockComponent();

	/** Toggles lock on/off */
	UFUNCTION(BlueprintCallable, Category = "Target Lock")
	void ToggleLockOn();

	/** Gets the current locked target */
	UFUNCTION(BlueprintPure, Category = "Target Lock")
	AActor* GetLockedTarget() const { return LockedTarget; }

protected:
	virtual void BeginPlay() override;

	/** Logic to find the best target in front of the player */
	AActor* FindBestTarget();

private:
	UPROPERTY()
	AActor* LockedTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Target Lock")
	float LockRange = 1500.0f;
};
