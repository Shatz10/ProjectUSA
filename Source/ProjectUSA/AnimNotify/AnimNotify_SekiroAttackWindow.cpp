// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify/AnimNotify_SekiroAttackWindow.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"

UAnimNotify_SekiroAttackWindow::UAnimNotify_SekiroAttackWindow()
{
	bIsWindowStart = true;
}

void UAnimNotify_SekiroAttackWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	if (!MeshComp || !EventTag.IsValid())
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

	// Send GameplayEvent
	FGameplayEventData EventData;
	EventData.Instigator = Owner;
	EventData.Target = Owner;

	ASC->HandleGameplayEvent(EventTag, &EventData);

	UE_LOG(LogTemp, Verbose, TEXT("Attack Window Notify: %s - %s"), 
		*EventTag.ToString(), 
		bIsWindowStart ? TEXT("Start") : TEXT("End"));
}
