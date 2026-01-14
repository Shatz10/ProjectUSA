// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroHitReaction.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UGA_SekiroHitReaction::UGA_SekiroHitReaction()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bIsKnockedDown = false;

	// NOTE: In Blueprint, set:
	// HitStunTag = "State.HitStun"
	// KnockdownTag = "State.Knockdown"
}

void UGA_SekiroHitReaction::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get damage info from event data
	float Damage = 0.f;
	AActor* Attacker = nullptr;
	
	if (TriggerEventData)
	{
		Damage = TriggerEventData->EventMagnitude;
		Attacker = const_cast<AActor*>(TriggerEventData->Instigator.Get());
	}

	// Determine hit parameters
	bool bIsAirborne = Character->GetCharacterMovement() && Character->GetCharacterMovement()->IsFalling();
	EHitDamageLevel DamageLevel = DetermineDamageLevel(Damage, 100.f); // TODO: Get actual max health
	EHitDirection HitDir = DetermineHitDirection(Attacker, Character);

	// Select appropriate montage
	UAnimMontage* HitMontage = SelectHitMontage(DamageLevel, bIsAirborne, HitDir);

	if (!HitMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Apply appropriate tag
	bIsKnockedDown = (DamageLevel == EHitDamageLevel::Knockdown);
	
	if (GetAbilitySystemComponentFromActorInfo())
	{
		if (bIsKnockedDown && KnockdownTag.IsValid())
		{
			GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(KnockdownTag);
		}
		else if (HitStunTag.IsValid())
		{
			GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(HitStunTag);
		}
	}

	// Play hit reaction montage
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, HitMontage);
	
	PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_SekiroHitReaction::OnHitReactionCompleted);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_SekiroHitReaction::OnHitReactionCancelled);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_SekiroHitReaction::OnHitReactionCancelled);

	PlayMontageTask->ReadyForActivation();
}

void UGA_SekiroHitReaction::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Remove tags
	if (GetAbilitySystemComponentFromActorInfo())
	{
		if (HitStunTag.IsValid())
		{
			GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(HitStunTag);
		}
		if (KnockdownTag.IsValid())
		{
			GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(KnockdownTag);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SekiroHitReaction::OnHitReactionCompleted()
{
	// If knocked down, play get up animation
	if (bIsKnockedDown && GetUpMontage)
	{
		UAbilityTask_PlayMontageAndWait* GetUpTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, GetUpMontage);
		
		GetUpTask->OnCompleted.AddDynamic(this, &UGA_SekiroHitReaction::OnHitReactionCancelled);
		GetUpTask->OnCancelled.AddDynamic(this, &UGA_SekiroHitReaction::OnHitReactionCancelled);
		GetUpTask->OnInterrupted.AddDynamic(this, &UGA_SekiroHitReaction::OnHitReactionCancelled);

		GetUpTask->ReadyForActivation();
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_SekiroHitReaction::OnHitReactionCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

UAnimMontage* UGA_SekiroHitReaction::SelectHitMontage(EHitDamageLevel DamageLevel, bool bIsAirborne, EHitDirection Direction)
{
	if (DamageLevel == EHitDamageLevel::Knockdown && KnockdownMontage)
	{
		return KnockdownMontage;
	}

	const TMap<EHitDamageLevel, UAnimMontage*>& MontageMap = bIsAirborne ? AirHitMontages : GroundHitMontages;
	
	if (UAnimMontage* const* FoundMontage = MontageMap.Find(DamageLevel))
	{
		return *FoundMontage;
	}

	// Fallback to lighter damage level if specific level not found
	if (DamageLevel > EHitDamageLevel::Light)
	{
		return SelectHitMontage(static_cast<EHitDamageLevel>(static_cast<uint8>(DamageLevel) - 1), bIsAirborne, Direction);
	}

	return nullptr;
}

EHitDamageLevel UGA_SekiroHitReaction::DetermineDamageLevel(float Damage, float MaxHealth)
{
	float DamagePercent = Damage / FMath::Max(MaxHealth, 1.f);

	if (DamagePercent >= 0.5f)
	{
		return EHitDamageLevel::Knockdown;
	}
	else if (DamagePercent >= 0.3f)
	{
		return EHitDamageLevel::VeryHeavy;
	}
	else if (DamagePercent >= 0.2f)
	{
		return EHitDamageLevel::Heavy;
	}
	else if (DamagePercent >= 0.1f)
	{
		return EHitDamageLevel::Medium;
	}
	
	return EHitDamageLevel::Light;
}

EHitDirection UGA_SekiroHitReaction::DetermineHitDirection(AActor* Attacker, AActor* Victim)
{
	if (!Attacker || !Victim)
	{
		return EHitDirection::Front;
	}

	FVector ToAttacker = (Attacker->GetActorLocation() - Victim->GetActorLocation()).GetSafeNormal2D();
	FVector VictimForward = Victim->GetActorForwardVector().GetSafeNormal2D();

	float DotProduct = FVector::DotProduct(VictimForward, ToAttacker);
	float CrossProduct = FVector::CrossProduct(VictimForward, ToAttacker).Z;

	// Front/Back based on dot product
	if (FMath::Abs(DotProduct) > 0.707f) // ~45 degrees
	{
		return (DotProduct > 0) ? EHitDirection::Front : EHitDirection::Back;
	}
	
	// Left/Right based on cross product
	return (CrossProduct > 0) ? EHitDirection::Left : EHitDirection::Right;
}
