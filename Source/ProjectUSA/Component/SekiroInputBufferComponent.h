// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SekiroInputBufferComponent.generated.h"

/**
 * Input buffering component for Sekiro-style input queuing.
 * 
 * Logic:
 * - Records inputs during ability execution.
 * - Allows abilities to check and consume buffered inputs.
 * - Configurable buffer window (default 0.3s).
 * 
 * NOTE: Add this component to SekiroHeroCharacter Blueprint.
 * Abilities should call TryConsumeInput() to check for buffered inputs.
 */

USTRUCT(BlueprintType)
struct FBufferedInput
{
	GENERATED_BODY()

	UPROPERTY()
	FName InputName;

	UPROPERTY()
	float Timestamp;

	FBufferedInput()
		: InputName(NAME_None)
		, Timestamp(0.f)
	{}

	FBufferedInput(FName InName, float InTime)
		: InputName(InName)
		, Timestamp(InTime)
	{}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTUSA_API USekiroInputBufferComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USekiroInputBufferComponent();

	/** How long inputs remain in buffer (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Input Buffer")
	float BufferWindow = 0.3f;

	/** Record an input to the buffer */
	UFUNCTION(BlueprintCallable, Category = "Input Buffer")
	void RecordInput(FName InputName);

	/** Try to consume a buffered input (returns true if found and consumed) */
	UFUNCTION(BlueprintCallable, Category = "Input Buffer")
	bool TryConsumeInput(FName InputName);

	/** Clear all buffered inputs */
	UFUNCTION(BlueprintCallable, Category = "Input Buffer")
	void ClearBuffer();

	/** Check if an input is buffered without consuming it */
	UFUNCTION(BlueprintPure, Category = "Input Buffer")
	bool HasBufferedInput(FName InputName) const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TArray<FBufferedInput> InputBuffer;

	void CleanExpiredInputs();
};
