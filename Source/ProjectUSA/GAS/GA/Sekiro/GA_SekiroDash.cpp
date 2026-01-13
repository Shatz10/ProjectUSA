// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroDash.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"

UGA_SekiroDash::UGA_SekiroDash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	DashDistance = 400.f;
	DashDuration = 0.3f;
	bInvincibleDuringDash = true;

	// NOTE: In Blueprint, set DashingTag = "State.Dashing"
}

void UGA_SekiroDash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Apply Dashing tag
	if (DashingTag.IsValid() && GetAbilitySystemComponentFromActorInfo())
	{
		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(DashingTag);
	}

	// Get dash direction (input direction or forward)
	FVector DashDirection = Character->GetActorForwardVector();
	
	// Try to get input direction from character
	// NOTE: This assumes your character has a way to get input direction
	// You may need to adjust based on your character implementation
	if (Character->GetCharacterMovement() && Character->GetCharacterMovement()->GetLastInputVector().SizeSquared() > 0.01f)
	{
		DashDirection = Character->GetCharacterMovement()->GetLastInputVector();
		DashDirection.Z = 0.f;
		DashDirection.Normalize();
	}

	// Calculate launch velocity
	float LaunchSpeed = DashDistance / DashDuration;
	FVector LaunchVelocity = DashDirection * LaunchSpeed;

	// Launch character
	Character->LaunchCharacter(LaunchVelocity, true, true);

	// Play dash montage
	if (DashMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DashMontage);
		
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SekiroDash::OnDashCompleted);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SekiroDash::OnDashCompleted);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SekiroDash::OnDashCompleted);

		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		// If no montage, end after duration
		FTimerHandle DashTimer;
		GetWorld()->GetTimerManager().SetTimer(DashTimer, this, &UGA_SekiroDash::OnDashCompleted, DashDuration, false);
	}
}

void UGA_SekiroDash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Remove Dashing tag
	if (DashingTag.IsValid() && GetAbilitySystemComponentFromActorInfo())
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(DashingTag);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SekiroDash::OnDashCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
