// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//Required to set capsule focus
UENUM(BlueprintType)
enum class EUSACharacterCapsulePivot : uint8
{
	Top UMETA(DisplayName = "Top"),
	Center UMETA(DisplayName = "Center"),
	Bottom UMETA(DisplayName = "Bottom"),
};



//Movements required for character actions
//Movement (None / Simple Move to Position / Walk / Launch) -> Select only one
//Animation (Montage / Anim Blueprint / Both) -> Open All
//Creation (with/without) -> Select only one
//Direction (none / input direction / red)

UENUM(BlueprintType)
enum class ECharacterActionEndType : uint8
{
	None UMETA(DisplayName = "None"),
	WaitTime UMETA(DisplayName = "WaitTime"),
	WaitTagAdded UMETA(DisplayName = "WaitTagAdded"),
	WaitTagRemoved UMETA(DisplayName = "WaitTagRemoved"),
};

UENUM(BlueprintType)
enum class ECharacterActionMoveType : uint8
{
	None UMETA(DisplayName = "None"),
	Move UMETA(DisplayName = "Move"),
	Walk UMETA(DisplayName = "Walk"),
	Launch UMETA(DisplayName = "Launch"),
	Custom UMETA(DisplayName = "Custom"),
	//MoveToTarget UMETA(DisplayName = "MoveToTarget"), -> Because it has high priority, it is set aside as variables.
};

UENUM(BlueprintType)
enum class ECharacterActionDirectionType : uint8
{
	None UMETA(DisplayName = "None"),
	Input UMETA(DisplayName = "Input"),
	Target UMETA(DisplayName = "Target"),
	//Damage UMETA(DisplayName = "Damage"), <- first processed in CharacterBase
};

// ========================================================================

//UENUM(BlueprintType)
//enum class EUSAWeaponType : uint8
//{
//	None = 0 UMETA(DisplayName = "None"),
//	First = 1 UMETA(DisplayName = "First"),
//	Second = 2 UMETA(DisplayName = "Second"),
//};


//TODO: Decide whether to use or delete later
//UENUM(BlueprintType)
//enum class ECharacterTeam : uint8
//{
//	None UMETA(DisplayName = "None"),
//	Player UMETA(DisplayName = "Player"),
//	Enemy UMETA(DisplayName = "Enemy"),
//};

// =================================================================

UENUM(BlueprintType)
enum class ETargetablePivotType : uint8
{
	Bottom UMETA(DisplayName = "Bottom"),
	Center UMETA(DisplayName = "Center"),
	Top UMETA(DisplayName = "Top"),
};
