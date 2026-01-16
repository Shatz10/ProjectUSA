// Fill out your copyright notice in the Description page of Project Settings.


#include "Trigger/USALevelSequenceBegin.h"

#include "Components/TextRenderComponent.h"

#include "GameMode/USAGameModeBase.h"
#include "GameState/USAGameStateBase.h"

#include "LevelSequenceActor.h"


AUSALevelSequenceBegin::AUSALevelSequenceBegin()
{
	if (IsValid(LevelSequenceTriggerTextComponent))
	{
		LevelSequenceTriggerTextComponent->SetText(FText::FromString("Level Sequence Begin"));
	}
}

void AUSALevelSequenceBegin::BeginPlay()
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AUSALevelSequenceBegin::PlayBeginLevelSequence);
}

void AUSALevelSequenceBegin::PlayBeginLevelSequence()
{
	if (bIsActivated == true)
	{
		return;
	}

	//local
	if (bIsLocalEvent)
	{
		AUSAGameStateBase* USAGameStateBase = Cast <AUSAGameStateBase>(GetWorld()->GetGameState());

		if (IsValid(USAGameStateBase) == false)
		{
			return;
		}

		USAGameStateBase->PlayLevelSequenceLocal(LevelSequenceActor);
	}
	//multi
	else
	{
		AUSAGameModeBase* USAGameModeBase = Cast<AUSAGameModeBase>(GetWorld()->GetAuthGameMode());

		if (IsValid(USAGameModeBase) == false)
		{
			return;
		}

		USAGameModeBase->PlayLevelSequenceToAllPlayer(LevelSequenceActor);
	}

	bIsActivated = true;
}
