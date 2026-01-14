// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroJump.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UGA_SekiroJump::UGA_SekiroJump()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	JumpHeight = 600.f;
	JumpForwardSpeed = 400.f;
	InPlaceJumpMultiplier = 0.2f;
	CurrentJumpDirection = EJumpDirection::InPlace;

	// NOTE: In Blueprint, set JumpingTag = "State.Jumping"
}

void UGA_SekiroJump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !Character->GetCharacterMovement())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Apply jumping tag
	if (JumpingTag.IsValid() && GetAbilitySystemComponentFromActorInfo())
	{
		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(JumpingTag);
	}

	// Determine jump direction
	FVector InputDirection = Character->GetCharacterMovement()->GetLastInputVector();
	CurrentJumpDirection = DetermineJumpDirection(InputDirection, Character->GetActorForwardVector());

	// Apply jump velocity
	ApplyJumpVelocity(Character, CurrentJumpDirection);

	// Play jump start montage
	if (JumpStartMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, JumpStartMontage);
		
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SekiroJump::OnJumpStartCompleted);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SekiroJump::OnMontageInterrupted);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SekiroJump::OnMontageInterrupted);

		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		// No montage, go straight to loop
		OnJumpStartCompleted();
	}
}

void UGA_SekiroJump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Remove jumping tag
	if (JumpingTag.IsValid() && GetAbilitySystemComponentFromActorInfo())
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(JumpingTag);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SekiroJump::OnJumpStartCompleted()
{
	// Transition to loop state
	if (JumpLoopMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, JumpLoopMontage);
		
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SekiroJump::OnJumpLoopCompleted);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SekiroJump::OnMontageInterrupted);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SekiroJump::OnMontageInterrupted);

		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		// Wait for landing
		ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
		if (Character && Character->GetCharacterMovement())
		{
			// Check for landing in tick or use a timer
			OnJumpLoopCompleted();
		}
	}
}

void UGA_SekiroJump::OnJumpLoopCompleted()
{
	// Check if character has landed
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !Character->GetCharacterMovement())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// If still in air, keep looping
	if (Character->GetCharacterMovement()->IsFalling())
	{
		OnJumpStartCompleted(); // Loop back
		return;
	}

	// Landed - play end montage
	if (JumpEndMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, JumpEndMontage);
		
		PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SekiroJump::OnJumpEndCompleted);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SekiroJump::OnMontageInterrupted);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SekiroJump::OnMontageInterrupted);

		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		OnJumpEndCompleted();
	}
}

void UGA_SekiroJump::OnJumpEndCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SekiroJump::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

EJumpDirection UGA_SekiroJump::DetermineJumpDirection(const FVector& InputDirection, const FVector& ForwardVector)
{
	if (InputDirection.SizeSquared() < 0.01f)
	{
		return EJumpDirection::InPlace;
	}

	FVector NormalizedInput = InputDirection.GetSafeNormal2D();
	FVector NormalizedForward = ForwardVector.GetSafeNormal2D();

	float DotProduct = FVector::DotProduct(NormalizedForward, NormalizedInput);
	float CrossProduct = FVector::CrossProduct(NormalizedForward, NormalizedInput).Z;

	// Determine 8-directional input
	float Angle = FMath::Atan2(CrossProduct, DotProduct);
	float AngleDegrees = FMath::RadiansToDegrees(Angle);

	// Normalize to 0-360
	if (AngleDegrees < 0)
	{
		AngleDegrees += 360.f;
	}

	// Map to 8 directions (45 degree segments)
	if (AngleDegrees < 22.5f || AngleDegrees >= 337.5f)
		return EJumpDirection::Forward;
	else if (AngleDegrees < 67.5f)
		return EJumpDirection::ForwardRight;
	else if (AngleDegrees < 112.5f)
		return EJumpDirection::Right;
	else if (AngleDegrees < 157.5f)
		return EJumpDirection::BackRight;
	else if (AngleDegrees < 202.5f)
		return EJumpDirection::Back;
	else if (AngleDegrees < 247.5f)
		return EJumpDirection::BackLeft;
	else if (AngleDegrees < 292.5f)
		return EJumpDirection::Left;
	else
		return EJumpDirection::ForwardLeft;
}

void UGA_SekiroJump::ApplyJumpVelocity(ACharacter* Character, EJumpDirection Direction)
{
	if (!Character || !Character->GetCharacterMovement())
	{
		return;
	}

	FVector JumpVelocity = FVector::ZeroVector;
	JumpVelocity.Z = JumpHeight;

	// Apply horizontal velocity based on direction
	float HorizontalSpeed = (Direction == EJumpDirection::InPlace) ? JumpForwardSpeed * InPlaceJumpMultiplier : JumpForwardSpeed;

	FVector HorizontalDirection = FVector::ZeroVector;
	switch (Direction)
	{
	case EJumpDirection::Forward:
		HorizontalDirection = Character->GetActorForwardVector();
		break;
	case EJumpDirection::ForwardRight:
		HorizontalDirection = (Character->GetActorForwardVector() + Character->GetActorRightVector()).GetSafeNormal();
		break;
	case EJumpDirection::Right:
		HorizontalDirection = Character->GetActorRightVector();
		break;
	case EJumpDirection::BackRight:
		HorizontalDirection = (-Character->GetActorForwardVector() + Character->GetActorRightVector()).GetSafeNormal();
		break;
	case EJumpDirection::Back:
		HorizontalDirection = -Character->GetActorForwardVector();
		break;
	case EJumpDirection::BackLeft:
		HorizontalDirection = (-Character->GetActorForwardVector() - Character->GetActorRightVector()).GetSafeNormal();
		break;
	case EJumpDirection::Left:
		HorizontalDirection = -Character->GetActorRightVector();
		break;
	case EJumpDirection::ForwardLeft:
		HorizontalDirection = (Character->GetActorForwardVector() - Character->GetActorRightVector()).GetSafeNormal();
		break;
	default:
		break;
	}

	JumpVelocity += HorizontalDirection * HorizontalSpeed;

	// Launch character
	Character->LaunchCharacter(JumpVelocity, true, true);

	UE_LOG(LogTemp, Log, TEXT("Jump Direction: %d, Velocity: %s"), static_cast<int32>(Direction), *JumpVelocity.ToString());
}
