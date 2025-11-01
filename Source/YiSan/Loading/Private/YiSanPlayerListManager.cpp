// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "YiSanPlayerListManager.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "AYiSanPlayerState.h"
#include "GameLogging.h"
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

    PRINTLOG(TEXT("AYiSanPlayerListManager::BeginPlay - HasAuthority: %s"), HasAuthority() ? TEXT("true") : TEXT("false"));

    // Register this PlayerListManager with the GameInstance Subsystem
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UNetworkGameInstanceSubsystem* NetworkSubsystem = GameInstance->GetSubsystem<UNetworkGameInstanceSubsystem>())
        {
            PRINTLOG(TEXT("AYiSanPlayerListManager::BeginPlay - Registering with NetworkSubsystem"));
            NetworkSubsystem->SetPlayerListManager(this);
        }
    }
    // Do nothing on BeginPlay, wait for external trigger
}

void AYiSanPlayerListManager::UpdatePlayerListAndBroadcast()
{
    if (!HasAuthority()) return;

    AGameStateBase* GameState = GetWorld()->GetGameState();
    if (!GameState)
        return;

    // PlayerArray를 PlayerIndex 기준으로 정렬
    GameState->PlayerArray.Sort([](const APlayerState& A, const APlayerState& B)
    {
        const AYiSanPlayerState* PSA = Cast<const AYiSanPlayerState>(&A);
        const AYiSanPlayerState* PSB = Cast<const AYiSanPlayerState>(&B);

        if (!PSA || !PSB)
        {
            // nullptr 대비 – 그냥 이름으로라도 안정 정렬
            return A.GetPlayerName() < B.GetPlayerName();
        }

        return PSA->PlayerIndex < PSB->PlayerIndex;
    });
    
    TArray<FString> CurrentPlayerInfo;
    
    for (int32 i = 0; i < GameState->PlayerArray.Num(); ++i)
    {
        APlayerState* ps = GameState->PlayerArray[i];
        if (AYiSanPlayerState* YiSanPS = Cast<AYiSanPlayerState>(ps))
        {
            APlayerController* PC = GetWorld()->GetFirstPlayerController(); 
            AYiSanPlayerState* LocalPS = PC ? PC->GetPlayerState<AYiSanPlayerState>() : nullptr;
            
            FString ParseString = FString::Printf(TEXT("%d:%s"), YiSanPS->PlayerIndex, *YiSanPS->Nickname);
 
            CurrentPlayerInfo.Add(ParseString);
        }
    }


    PlayerList = CurrentPlayerInfo;
    PRINTLOG(TEXT("AYiSanPlayerListManager: PlayerList updated. Current players: %s"), *FString::Join(PlayerList, TEXT(", ")));
    OnRep_PlayerList();
}

void AYiSanPlayerListManager::OnRep_PlayerList()
{
    PRINTLOG(TEXT("AYiSanPlayerListManager::OnRep_PlayerList - PlayerList replicated with %d players"), PlayerList.Num());

    // On clients, notify via NetworkSubsystem if available
    if (!HasAuthority())
    {
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UNetworkGameInstanceSubsystem* NetworkSubsystem = GameInstance->GetSubsystem<UNetworkGameInstanceSubsystem>())
            {
                PRINTLOG(TEXT("AYiSanPlayerListManager::OnRep_PlayerList - Notifying NetworkSubsystem (Client)"));
                NetworkSubsystem->OnPlayerListUpdated.Broadcast(this->PlayerList);
            }
            else
            {
                PRINTLOG(TEXT("AYiSanPlayerListManager::OnRep_PlayerList - NetworkSubsystem not available yet (Client)"));
            }
        }
    }

    // Also broadcast directly for any local subscribers
    PRINTLOG(TEXT("AYiSanPlayerListManager::OnRep_PlayerList - Broadcasting to direct delegates"));
    OnPlayerListUpdated.Broadcast(this->PlayerList);
}

void AYiSanPlayerListManager::RequestRefresh()
{
    PRINTLOG(TEXT("AYiSanPlayerListManager::RequestRefresh - HasAuthority: %s"), HasAuthority() ? TEXT("true") : TEXT("false"));

    // If on client, send RPC to server to refresh
    if (!HasAuthority())
    {
        PRINTLOG(TEXT("AYiSanPlayerListManager::RequestRefresh - Client sending ServerRPC_RequestRefresh"));
        ServerRPC_RequestRefresh();
    }
    else // If on server, just update the list
    {
        PRINTLOG(TEXT("AYiSanPlayerListManager::RequestRefresh - Server updating player list directly"));
        UpdatePlayerListAndBroadcast();
    }
}

void AYiSanPlayerListManager::ServerRPC_RequestRefresh_Implementation()
{
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AYiSanPlayerListManager::UpdatePlayerListAndBroadcast, 0.2f, false);
}
