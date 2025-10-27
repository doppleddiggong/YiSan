// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "YiSanPlayerListManager.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

AYiSanPlayerListManager::AYiSanPlayerListManager()
{
    bReplicates = true;
    bAlwaysRelevant = true; // 이 줄을 추가합니다.          
    SetReplicatingMovement(false);
    PrimaryActorTick.bCanEverTick = false;
}

void AYiSanPlayerListManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AYiSanPlayerListManager, PlayerList);
}

void AYiSanPlayerListManager::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(PollingTimer, this, &AYiSanPlayerListManager::PollPlayerList, 1.0f, true);
    }
}

void AYiSanPlayerListManager::PollPlayerList()
{
    if (!HasAuthority())
    {
        return;
    }

    AGameStateBase* GameState = GetWorld()->GetGameState();
    if (!GameState)
    {
        return;
    }

    TArray<FString> CurrentPlayerNames;
    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        if (PlayerState)
        {
            CurrentPlayerNames.Add(PlayerState->GetPlayerName());
        }
    }

    // Sort arrays to compare them regardless of order
    CurrentPlayerNames.Sort();
    TArray<FString> SortedPlayerList = PlayerList;
    SortedPlayerList.Sort();

    if (CurrentPlayerNames != SortedPlayerList)
    {
        PlayerList = CurrentPlayerNames;
        // On server, manually trigger the update if needed for host UI
        OnRep_PlayerList(); 
    }
}

void AYiSanPlayerListManager::OnRep_PlayerList()
{
    OnPlayerListUpdated.Broadcast(PlayerList);
}
