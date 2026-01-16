// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/USAPlayerController.h"

#include "Character/USACharacterBase.h"
#include "Character/USACharacterPlayer.h"

#include "Weapon/USAWeaponBase.h"
#include "Item/USAItemBase.h"

#include "HUD/USAHUD.h"

#include "GameInstance/USAGameInstance.h"

#include "Player/USAPlayerState.h"

#include "Kismet/GameplayStatics.h"

#include "Trigger/USALevelSequenceBegin.h"


void AUSAPlayerController::BeginPlay()
{
    Super::BeginPlay();

    AUSACharacterPlayer* USACharacterOwner = Cast<AUSACharacterPlayer>(GetPawn());
    AUSAHUD* USAHUD = Cast<AUSAHUD>(GetHUD());
    UUSAGameInstance* USAGameInstance = Cast <UUSAGameInstance>(GetGameInstance());
    AUSAPlayerState* USAPlayerState = GetPlayerState<AUSAPlayerState>();

// Get HUD
    LocalUSAHUD = Cast<AUSAHUD>(GetHUD());
}

void AUSAPlayerController::BeginPlayingState()
{
    Super::BeginPlayingState();

    AUSACharacterPlayer* USACharacterOwner = Cast<AUSACharacterPlayer>(GetPawn());
    AUSAHUD* USAHUD = Cast<AUSAHUD>(GetHUD());
    UUSAGameInstance* USAGameInstance = Cast <UUSAGameInstance>(GetGameInstance());
    AUSAPlayerState* USAPlayerState = GetPlayerState<AUSAPlayerState>();

// Called at startup
    if (IsLocalController() == true)
    {
        if (USACharacterOwner)
        {
            USACharacterOwner->InitPlayerController();
        }

        if (USACharacterOwner)
        {
            USACharacterOwner->SetCurrentWeaponsUsingStartWeaponClassList();
        }

        if (USACharacterOwner && USAHUD)
        {
            // USAHUD->InitPlayerHUD(USACharacterOwner);
        }

// If there is no first intro
        if (IsValid(UGameplayStatics::GetActorOfClass(GetWorld(), AUSALevelSequenceBegin::StaticClass())) == false)
        {
            //USAHUD->PlayUserWidgetAnimation_Panel(true, false);
        }
    }
}
