// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SekiroHeroCharacter.h"
#include "GAS/AttributeSet/USAAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Component/SekiroInputBufferComponent.h"
#include "Component/SekiroTargetLockComponent.h"
#include "GAS/GA/Sekiro/GA_SekiroProstheticTool.h"
#include "TimerManager.h"
#include "GameplayTagsManager.h"

ASekiroHeroCharacter::ASekiroHeroCharacter()
{
	// Defaults for Sekiro gameplay
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Create input buffer component
	InputBufferComponent = CreateDefaultSubobject<USekiroInputBufferComponent>(TEXT("InputBufferComponent"));

	// Create Target Lock-on component
	TargetLockComponent = CreateDefaultSubobject<USekiroTargetLockComponent>(TEXT("TargetLockComponent"));
}

void ASekiroHeroCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const UUSAAttributeSet* USAAttributeSet  = Cast<UUSAAttributeSet>(ASC->GetSet<UUSAAttributeSet>()))
	{
		USAAttributeSet->OnPostureBroken.AddDynamic(this, &ASekiroHeroCharacter::OnPostureBroken);
	}
}

void ASekiroHeroCharacter::SetupGAS()
{
	Super::SetupGAS();

	// Here we would grant initial abilities like Deflect, Attack, etc.
	// NOTE: You should add GA_SekiroDeflect and GA_SekiroAttack to the 'GameplayAbilities_Start' array
	// in the Blueprint derived from this class (ASekiroHeroCharacter).
	// The Base class (AUSACharacterBase) handles granting abilities in that array automatically.
}

float ASekiroHeroCharacter::GetCurrentPosture() const
{
	if (const UUSAAttributeSet* USAAttributeSet  = Cast<UUSAAttributeSet>(ASC->GetSet<UUSAAttributeSet>()))
	{
		return USAAttributeSet->GetCurrentPosture();
	}
	return 0.0f;
}

float ASekiroHeroCharacter::GetMaxPosture() const
{
	if (const UUSAAttributeSet* USAAttributeSet  = Cast<UUSAAttributeSet>(ASC->GetSet<UUSAAttributeSet>()))
	{
		return USAAttributeSet->GetMaxPosture();
	}
	return 0.0f;
}

float ASekiroHeroCharacter::GetCurrentSpiritEmblems() const
{
	if (const UUSAAttributeSet* USAAttributeSet  = Cast<UUSAAttributeSet>(ASC->GetSet<UUSAAttributeSet>()))
	{
		return USAAttributeSet->GetCurrentSpiritEmblems();
	}
	return 0.0f;
}

float ASekiroHeroCharacter::GetMaxSpiritEmblems() const
{
	if (const UUSAAttributeSet* USAAttributeSet  = Cast<UUSAAttributeSet>(ASC->GetSet<UUSAAttributeSet>()))
	{
		return USAAttributeSet->GetMaxSpiritEmblems();
	}
	return 0.0f;
}

void ASekiroHeroCharacter::OnPostureBroken()
{
	UE_LOG(LogTemp, Warning, TEXT("SekiroHeroCharacter::OnPostureBroken - Posture Broken!"));
	
	// Apply State.PostureBroken tag
	if (UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponent())
	{
		FGameplayTag PostureBrokenTag = FGameplayTag::RequestGameplayTag(FName("State.PostureBroken"));
		if (PostureBrokenTag.IsValid())
		{
			ASC->AddLooseGameplayTag(PostureBrokenTag);
			
			// Remove tag after a duration (e.g., 3 seconds)
			FTimerHandle PostureBrokenTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(PostureBrokenTimerHandle, [this, AbilitySystem, PostureBrokenTag]()
			{
				if (ASC)
				{
					ASC->RemoveLooseGameplayTag(PostureBrokenTag);
				}
			}, 3.0f, false);
		}
	}
}

void ASekiroHeroCharacter::InputPressGameplayAbilityByInputID(int32 InputID)
{
	// Record input in buffer for potential consumption later
	if (InputBufferComponent)
	{
		FName InputName = FName(*FString::Printf(TEXT("Input_%d"), InputID));
		InputBufferComponent->RecordInput(InputName);
	}

	// Try to activate ability immediately via base class
	Super::InputPressGameplayAbilityByInputID(InputID);
}

void ASekiroHeroCharacter::TryActivateAbilityWithBuffer(int32 InputID, FName InputName)
{
	// Check if input is buffered and try to consume it
	if (InputBufferComponent && InputBufferComponent->TryConsumeInput(InputName))
	{
		UE_LOG(LogTemp, Log, TEXT("Activating ability from buffered input: %s"), *InputName.ToString());
		Super::InputPressGameplayAbilityByInputID(InputID);
	}
}

void ASekiroHeroCharacter::CycleProstheticTool()
{
	if (EquippedTools.Num() == 0) return;

	CurrentToolIndex = (CurrentToolIndex + 1) % EquippedTools.Num();
	
	UE_LOG(LogTemp, Log, TEXT("Prosthetic Tool Switched to index: %d"), CurrentToolIndex);
	
	// Send Event to UI
	if (ASC)
	{
		FGameplayEventData Payload;
		Payload.Instigator = this;
		Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("GameplayEvent.UI.ToolSwitched"));
		ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
}

TSubclassOf<UGA_SekiroProstheticTool> ASekiroHeroCharacter::GetCurrentProstheticTool() const
{
	if (EquippedTools.IsValidIndex(CurrentToolIndex))
	{
		return EquippedTools[CurrentToolIndex];
	}
	return nullptr;
}

float ASekiroHeroCharacter::GetVisibilityMultiplier() const
{
	if (!ASC) return 1.0f;

	// In Sekiro, crouching significantly reduces visibility distance.
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Crouching"))))
	{
		// 50% reduction in visibility distance when crouching
		return 0.5f;
	}

	// 100% (Normal) visibility
	return 1.0f;
}

bool ASekiroHeroCharacter::CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor, const bool* bWasVisible, int32* UserData) const
{
	// Default visibility logic: aim for the chest/head
	OutSeenLocation = GetActorLocation() + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	OutSightStrength = GetVisibilityMultiplier();

	// If visibility is 0, we can't be seen
	if (OutSightStrength <= 0.01f)
	{
		return false;
	}

	// Perform standard line trace to check for LOS
	FHitResult HitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CanBeSeenFrom), true, IgnoreActor);
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, ObserverLocation, OutSeenLocation, ECC_Visibility, Params);
	NumberOfLoSChecksPerformed++;

	return !bHit || (HitResult.GetActor() == this);
}
