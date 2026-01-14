// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SekiroHeroCharacter.h"
#include "GAS/AttributeSet/USAAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Component/SekiroInputBufferComponent.h"
#include "TimerManager.h"
#include "GameplayTagsManager.h"

ASekiroHeroCharacter::ASekiroHeroCharacter()
{
	// Defaults for Sekiro gameplay
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Create input buffer component
	InputBufferComponent = CreateDefaultSubobject<USekiroInputBufferComponent>(TEXT("InputBufferComponent"));
}

void ASekiroHeroCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UUSAAttributeSet* USAAttributeSet = Cast<UUSAAttributeSet>(GetAttributeSet()))
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
	if (const UUSAAttributeSet* USAAttributeSet = Cast<UUSAAttributeSet>(GetAttributeSet()))
	{
		return USAAttributeSet->GetCurrentPosture();
	}
	return 0.0f;
}

float ASekiroHeroCharacter::GetMaxPosture() const
{
	if (const UUSAAttributeSet* USAAttributeSet = Cast<UUSAAttributeSet>(GetAttributeSet()))
	{
		return USAAttributeSet->GetMaxPosture();
	}
	return 0.0f;
}

void ASekiroHeroCharacter::OnPostureBroken()
{
	UE_LOG(LogTemp, Warning, TEXT("SekiroHeroCharacter::OnPostureBroken - Posture Broken!"));
	
	// Apply State.PostureBroken tag
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		FGameplayTag PostureBrokenTag = FGameplayTag::RequestGameplayTag(FName("State.PostureBroken"));
		if (PostureBrokenTag.IsValid())
		{
			ASC->AddLooseGameplayTag(PostureBrokenTag);
			
			// Remove tag after a duration (e.g., 3 seconds)
			FTimerHandle PostureBrokenTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(PostureBrokenTimerHandle, [this, ASC, PostureBrokenTag]()
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
