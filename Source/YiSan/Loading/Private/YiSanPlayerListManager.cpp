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
    bAlwaysRelevant = true;
    PrimaryActorTick.bCanEverTick = false;

    SetReplicatingMovement(false);
}

void AYiSanPlayerListManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AYiSanPlayerListManager, PlayerList);
}

void AYiSanPlayerListManager::BeginPlay()
{
    Super::BeginPlay();

    if (auto GI = GetGameInstance())
    {
        if (auto NetworkSubsystem = GI->GetSubsystem<UNetworkGameInstanceSubsystem>())
        {
            NetworkSubsystem->SetPlayerListManager(this);
        }
    }
}

void AYiSanPlayerListManager::BroadcastPlayerList()
{
    if (!HasAuthority())
        return;

    AGameStateBase* GameState = GetWorld()->GetGameState();
    if (!GameState)
        return;

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
    
    TArray<FString> PlayerInfoList;
    
    for (int32 i = 0; i < GameState->PlayerArray.Num(); ++i)
    {
        if (auto PS = Cast<AYiSanPlayerState>(GameState->PlayerArray[i]))
        {
            PlayerInfoList.Add(FString::Printf(TEXT("%d:%s"), PS->PlayerIndex, *PS->Nickname));
        }
    }


    PlayerList = PlayerInfoList;
    PRINTLOG(TEXT("AYiSanPlayerListManager: PlayerList updated. Current players: %s"),
        *FString::Join(PlayerList, TEXT(", ")));

    OnRep_PlayerList();
}

void AYiSanPlayerListManager::OnRep_PlayerList()
{
    if (!HasAuthority())
    {
        if (auto GI = GetGameInstance())
        {
            if (auto NetworkSubsystem = GI->GetSubsystem<UNetworkGameInstanceSubsystem>())
            {
                NetworkSubsystem->OnPlayerListUpdated.Broadcast(this->PlayerList);
            }
        }
    }

    OnPlayerListUpdated.Broadcast(this->PlayerList);
}

void AYiSanPlayerListManager::RequestRefresh()
{
    if (HasAuthority())
    {
        BroadcastPlayerList();
    }
    else
    {
        ServerRPC_UpdatePlayerList();
    }
}

void AYiSanPlayerListManager::ServerRPC_UpdatePlayerList_Implementation()
{
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AYiSanPlayerListManager::BroadcastPlayerList, 0.2f, false);
}
