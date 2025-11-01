// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "YiSanPlayerListManager.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "AYiSanPlayerState.h"
#include "ALobbyGameMode.h"
#include "GameLogging.h"
#include "UStartWidget.h"
#include "UNetworkGameInstanceSubsystem.h" // Added include
#include "Engine/GameInstance.h" // Added include for GameInstance

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

    // Register this PlayerListManager with the GameInstance Subsystem
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UNetworkGameInstanceSubsystem* NetworkSubsystem = GameInstance->GetSubsystem<UNetworkGameInstanceSubsystem>())
        {
            NetworkSubsystem->SetPlayerListManager(this);
        }
    }
    // Do nothing on BeginPlay, wait for external trigger
}

void AYiSanPlayerListManager::UpdatePlayerListAndBroadcast()
{
    if (!HasAuthority()) return;
    
    AGameStateBase* GameState = GetWorld()->GetGameState();
    if (!GameState) return;
    
    TArray<FString> CurrentPlayerInfo;
    for (int32 i = 0; i < GameState->PlayerArray.Num(); ++i)
    {
        APlayerState* ps = GameState->PlayerArray[i];
        if (AYiSanPlayerState* YiSanPS = Cast<AYiSanPlayerState>(ps))
        {
            if (!YiSanPS->Nickname.IsEmpty())
            {
                APlayerController* PC = Cast<APlayerController>(ps->GetOwner());
                bool bIsHost = PC ? PC->HasAuthority() : false;
                bool bIsReady = true; // Placeholder for ready status

                FString PlayerInfoString = FString::Printf(TEXT("%s:%s:%s:%d"), *YiSanPS->Nickname, bIsHost ? TEXT("true") : TEXT("false"), bIsReady ? TEXT("true") : TEXT("false"), i);
                CurrentPlayerInfo.Add(PlayerInfoString);
            }
        }
    }
    
    // Sort arrays to compare them regardless of order
    CurrentPlayerInfo.Sort();
    TArray<FString> SortedPlayerList = PlayerList;
    SortedPlayerList.Sort();

    if (CurrentPlayerInfo != SortedPlayerList)
    {
        PlayerList = CurrentPlayerInfo;
        PRINTLOG(TEXT("AYiSanPlayerListManager: PlayerList updated. Current players: %s"), *FString::Join(PlayerList, TEXT(", ")));
        // On server, manually trigger the update if needed for host UI
        OnRep_PlayerList(); 
    }
}

void AYiSanPlayerListManager::OnRep_PlayerList()
{
    OnPlayerListUpdated.Broadcast(this->PlayerList);
}

void AYiSanPlayerListManager::RequestRefresh()
{
    // If on client, send RPC to server to refresh
    if (!HasAuthority())
    {
        ServerRPC_RequestRefresh();
    }
    else // If on server, just update the list
    {
        UpdatePlayerListAndBroadcast();
    }
}

void AYiSanPlayerListManager::ServerRPC_RequestRefresh_Implementation()
{
    UpdatePlayerListAndBroadcast();
}
