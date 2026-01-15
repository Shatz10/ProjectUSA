// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroCrouch.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"

UGA_SekiroCrouch::UGA_SekiroCrouch()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	CrouchSpeedMultiplier = 0.5f;

	// NOTE: In Blueprint, set CrouchingTag = "State.Crouching"
}

void UGA_SekiroCrouch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !Character->GetCharacterMovement())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Toggle logic: If already crouching, end ability. Else, start crouching.
	if (ASC->HasMatchingGameplayTag(CrouchingTag))
	{
		// This should be handled by the ability triggering logic (Cancel if active)
		// But for a simple toggle we can do it here if it's not a 'Toggle' ability.
		// However, GAS usually handles this by having the ability active while crouching.
	}

	// Apply Tag
	ASC->AddLooseGameplayTag(CrouchingTag);

	// Reduce Speed
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	OriginalMaxWalkSpeed = Movement->MaxWalkSpeed;
	Movement->MaxWalkSpeed = OriginalMaxWalkSpeed * CrouchSpeedMultiplier;

	UE_LOG(LogTemp, Log, TEXT("Sekiro Crouching started. Speed reduced."));
	
	// Ability stays active while crouching
}

void UGA_SekiroCrouch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(CrouchingTag);
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
	}

	UE_LOG(LogTemp, Log, TEXT("Sekiro Crouching ended. Speed restored."));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
