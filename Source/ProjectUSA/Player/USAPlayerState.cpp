// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/USAPlayerState.h"

#include "AbilitySystemComponent.h"

#include "GAS/AttributeSet/USAAttributeSet.h"

#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"

#include "Player/USAPlayerController.h"
#include "HUD/USAHUD.h"

#include "Character/USACharacterBase.h"

#include "GameInstance/USAGameInstance.h"

#include "Net/UnrealNetwork.h"


AUSAPlayerState::AUSAPlayerState()
{
// If you change the names of the components below, they don't seem to be recognized.
    ASC = CreateDefaultSubobject <UAbilitySystemComponent>(TEXT("Ability System Component"));
    AttributeSet = CreateDefaultSubobject <UUSAAttributeSet>(TEXT("USA Attribute Set"));

//multi activation
    if (ASC != nullptr)
    {
        ASC->SetIsReplicated(true);
    }

    NetUpdateFrequency = 100.0f;
}

UAbilitySystemComponent* AUSAPlayerState::GetAbilitySystemComponent() const
{
    return ASC;
}

float AUSAPlayerState::GetLookSensitivityRatio()
{
    if (bIsUsingGamepad == true) 
    {
        return LookSensitivityGamepadRatio;
    }
    
    return LookSensitivityMouseRatio;
}

void AUSAPlayerState::SetIsUsingGamepad(bool InUsing)
{
    bIsUsingGamepad = InUsing;
}

void AUSAPlayerState::OnRep_PlayerName()
{
    Super::OnRep_PlayerName();

    AUSACharacterBase* USACharacter = Cast<AUSACharacterBase>(GetPawn());

    if (USACharacter)
    {
        USACharacter->ChangeCharacterName(GetPlayerName());
    }
}
