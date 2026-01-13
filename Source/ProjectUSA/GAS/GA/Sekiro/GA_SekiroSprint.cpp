// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroSprint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"

UGA_SekiroSprint::UGA_SekiroSprint()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	SprintSpeedMultiplier = 1.5f;
	OriginalMaxWalkSpeed = 0.f;

	// NOTE: In Blueprint, set SprintingTag = "State.Sprinting"
}

void UGA_SekiroSprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !Character->GetCharacterMovement())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Apply Sprinting tag
	if (SprintingTag.IsValid() && GetAbilitySystemComponentFromActorInfo())
	{
		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(SprintingTag);
	}

	// Store original speed and increase
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	OriginalMaxWalkSpeed = Movement->MaxWalkSpeed;
	Movement->MaxWalkSpeed = OriginalMaxWalkSpeed * SprintSpeedMultiplier;
}

void UGA_SekiroSprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Remove Sprinting tag
	if (SprintingTag.IsValid() && GetAbilitySystemComponentFromActorInfo())
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(SprintingTag);
	}

	// Restore original speed
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character && Character->GetCharacterMovement() && OriginalMaxWalkSpeed > 0.f)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SekiroSprint::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// End sprint when input is released
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
