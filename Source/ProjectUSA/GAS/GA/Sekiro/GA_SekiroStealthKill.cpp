// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroStealthKill.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UGA_SekiroStealthKill::UGA_SekiroStealthKill()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// NOTE: In Blueprint, set UnawareTag = "State.Unaware"
}

void UGA_SekiroStealthKill::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// Similar logic to GA_SekiroExecution but with stealth specific checks
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

AActor* UGA_SekiroStealthKill::FindExecutionTarget()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return nullptr;

	FVector Start = Character->GetActorLocation();
	FVector End = Start + (Character->GetActorForwardVector() * 200.f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Pawn, Params))
	{
		AActor* HitActor = HitResult.GetActor();
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(HitActor))
		{
			UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
			
			// CHECK 1: Is the enemy unaware?
			bool bIsUnaware = TargetASC && TargetASC->HasMatchingGameplayTag(UnawareTag);
			
			// CHECK 2: Are we behind them?
			bool bIsBehind = false;
			FVector ToTarget = (HitActor->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
			float Dot = FVector::DotProduct(HitActor->GetActorForwardVector(), ToTarget);
			if (Dot > 0.5f) // Facing the same direction roughly
			{
				bIsBehind = true;
			}

			if (bIsUnaware || bIsBehind)
			{
				return HitActor;
			}
		}
	}

	return nullptr;
}
