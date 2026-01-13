// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/SekiroInputBufferComponent.h"
#include "Engine/World.h"

USekiroInputBufferComponent::USekiroInputBufferComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // Tick every 100ms to clean expired inputs
	BufferWindow = 0.3f;
}

void USekiroInputBufferComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USekiroInputBufferComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	CleanExpiredInputs();
}

void USekiroInputBufferComponent::RecordInput(FName InputName)
{
	if (InputName == NAME_None)
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	InputBuffer.Add(FBufferedInput(InputName, CurrentTime));

	UE_LOG(LogTemp, Verbose, TEXT("Input buffered: %s at time %f"), *InputName.ToString(), CurrentTime);
}

bool USekiroInputBufferComponent::TryConsumeInput(FName InputName)
{
	if (InputName == NAME_None)
	{
		return false;
	}

	// Find the input in buffer
	for (int32 i = InputBuffer.Num() - 1; i >= 0; --i)
	{
		if (InputBuffer[i].InputName == InputName)
		{
			// Check if still valid
			float CurrentTime = GetWorld()->GetTimeSeconds();
			if (CurrentTime - InputBuffer[i].Timestamp <= BufferWindow)
			{
				InputBuffer.RemoveAt(i);
				UE_LOG(LogTemp, Log, TEXT("Input consumed from buffer: %s"), *InputName.ToString());
				return true;
			}
		}
	}

	return false;
}

void USekiroInputBufferComponent::ClearBuffer()
{
	InputBuffer.Empty();
}

bool USekiroInputBufferComponent::HasBufferedInput(FName InputName) const
{
	if (InputName == NAME_None)
	{
		return false;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	
	for (const FBufferedInput& Input : InputBuffer)
	{
		if (Input.InputName == InputName)
		{
			if (CurrentTime - Input.Timestamp <= BufferWindow)
			{
				return true;
			}
		}
	}

	return false;
}

void USekiroInputBufferComponent::CleanExpiredInputs()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	
	// Remove expired inputs
	InputBuffer.RemoveAll([CurrentTime, this](const FBufferedInput& Input)
	{
		return (CurrentTime - Input.Timestamp) > BufferWindow;
	});
}
