// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"

//#include "Engine.h"


// running of function Clara and server information bringing macro
// UENUMthis No because directly conditional statement through information have to get it box
#define LOG_NETMODEINFO ((GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : ((GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))
#define LOG_NETMODEINFO_GAMEPLAYABILITY ((GetAvatarActorFromActorInfo()->GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : ((GetAvatarActorFromActorInfo()->GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))
#define LOG_NETMODEINFO_ABILITYTASK ((Ability->GetAvatarActorFromActorInfo()->GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : ((Ability->GetAvatarActorFromActorInfo()->GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))


//3_04
// in netmode Local and Remoteof Rolesecond bringing macro
#define LOG_LOCALROLEINFO *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), GetLocalRole()))
#define LOG_LOCALROLEINFO_GAMEPLAYABILITY *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), GetAvatarActorFromActorInfo()->GetLocalRole()))
#define LOG_LOCALROLEINFO_ABILITYTASK *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), Ability->GetAvatarActorFromActorInfo()->GetLocalRole()))

#define LOG_REMOTEROLEINFO *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), GetRemoteRole()))
#define LOG_REMOTEROLEINFO_GAMEPLAYABILITY *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), GetAvatarActorFromActorInfo()->GetRemoteRole()))
#define LOG_REMOTEROLEINFO_ABILITYTASK *(UEnum::GetValueAsString (TEXT("Engine.ENetRole"), Ability->GetAvatarActorFromActorInfo()->GetRemoteRole()))


// being called of function information bringing macro
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)


#define USA_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s] [%s/%s] %s %s"), \
LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

#define USA_LOG_GAMEPLAYABILITY(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s] [%s/%s] %s %s"), \
LOG_NETMODEINFO_GAMEPLAYABILITY, LOG_LOCALROLEINFO_GAMEPLAYABILITY, LOG_REMOTEROLEINFO_GAMEPLAYABILITY, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

#define USA_LOG_ABILITYTASK(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s] [%s/%s] %s %s"), \
LOG_NETMODEINFO_ABILITYTASK, LOG_LOCALROLEINFO_ABILITYTASK, LOG_REMOTEROLEINFO_ABILITYTASK, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
