// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Sekiro/GA_SekiroProsthetic_Shuriken.h"
#include "GAS/GA/Sekiro/SekiroShurikenProjectile.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UGA_SekiroProsthetic_Shuriken::UGA_SekiroProsthetic_Shuriken()
{
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Prosthetic.Shuriken")));
	SpiritEmblemCost = 1;
}

void UGA_SekiroProsthetic_Shuriken::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 1. Spawning logic - in a real game, this would be timed with the animation
	// For now, we'll spawn it shortly after activation or at the end of the montage
	
	// If we have no montage, spawn immediately
	if (!ToolMontage)
	{
		SpawnShuriken();
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_SekiroProsthetic_Shuriken::SpawnShuriken()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !ProjectileClass) return;

	FVector SpawnLocation = Character->GetActorLocation() + (Character->GetActorForwardVector() * 50.f) + FVector(0,0,50);
	FRotator SpawnRotation = Character->GetControlRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;

	GetWorld()->SpawnActor<ASekiroShurikenProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	
	UE_LOG(LogTemp, Log, TEXT("Shuriken Spawned!"));
}
