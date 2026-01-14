// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify/AnimNotify_SekiroParryWindow.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"

UAnimNotify_SekiroParryWindow::UAnimNotify_SekiroParryWindow()
{
	bAddTag = true;
	// NOTE: In Blueprint, set ParryTag = "State.PerfectParry"
}

void UAnimNotify_SekiroParryWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	if (!MeshComp || !ParryTag.IsValid())
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	// Try to get AbilitySystemComponent
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner);
	if (!ASI)
	{
		return;
	}

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Add or remove tag
	if (bAddTag)
	{
		ASC->AddLooseGameplayTag(ParryTag);
		UE_LOG(LogTemp, Verbose, TEXT("Perfect Parry Window: START"));
	}
	else
	{
		ASC->RemoveLooseGameplayTag(ParryTag);
		UE_LOG(LogTemp, Verbose, TEXT("Perfect Parry Window: END"));
	}
}
