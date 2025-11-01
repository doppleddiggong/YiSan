// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UPlayerWidget.h"

#include "AYiSanPlayerState.h"
#include "UNetworkGameInstanceSubsystem.h"
#include "Engine/GameInstance.h"
#include "Components/VerticalBox.h" 
#include "Blueprint/WidgetTree.h"
#include "GameLogging.h"

void UPlayerWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (auto pc = GetOwningPlayer())
    {
        if (!pc->IsLocalController())
        {
            // 로컬이 아니면 숨김
            SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // Get the Network Game Instance Subsystem
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UNetworkGameInstanceSubsystem* NetworkSubsystem = GameInstance->GetSubsystem<UNetworkGameInstanceSubsystem>())
        {
            PRINTLOG(TEXT("UPlayerWidget found the NetworkSubsystem!"));
            NetworkSubsystem->OnPlayerListUpdated.AddUObject(this, &UPlayerWidget::OnPlayerListUpdated);
            NetworkSubsystem->RequestPlayerListRefresh();
        }
    }
    else
    {
        PRINTLOG(TEXT("UPlayerWidget FAILED to find the NetworkSubsystem!"));
    }

    if (!PlayerListContainer)
    {
        PRINTLOG(TEXT("UPlayerWidget::PlayerListContainer is NULL! Check WBP_PlayerWidget binding."));
    }
}

void UPlayerWidget::UpdatePlayerList(const TArray<FString>& playerNames)
{
    PRINTLOG(TEXT("UPlayerWidget::UpdatePlayerList called with %d player(s)."), playerNames.Num());
    if (!PlayerListContainer)
        return;

    PlayerListContainer->ClearChildren(); // Clear existing entries
    PlayerListContainer->SetVisibility(ESlateVisibility::Visible);

    // ✅ 내 PlayerIndex 구하기
    int32 LocalPlayerIndex = -1;
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (AYiSanPlayerState* LocalPS = PC->GetPlayerState<AYiSanPlayerState>())
        {
            LocalPlayerIndex = LocalPS->PlayerIndex;
            PRINTLOG(TEXT("LocalPlayerIndex = %d"), LocalPlayerIndex);
        }
    }
    
    for (const FString& PlayerInfoString : playerNames)
    {
        TArray<FString> PlayerInfo;
        PlayerInfoString.ParseIntoArray(PlayerInfo, TEXT(":"), true);

        if (PlayerInfo.Num() == 2)
        {
            int32 PlayerIndex = FCString::Atoi(*PlayerInfo[0]);
            FString PlayerName = PlayerInfo[1];

            UPlayerListItem* PlayerListItem = CreatePlayerListItem(PlayerIndex, LocalPlayerIndex, PlayerName );
            if (PlayerListItem)
            {
                PlayerListContainer->AddChild(PlayerListItem);
            }
        }
    }
}

UPlayerListItem* UPlayerWidget::CreatePlayerListItem(const int32 InPlayerIndex, const int32 LocalPlayerIndex, const FString& InPlayerName)
{
    PRINTLOG(TEXT("UPlayerWidget::CreatePlayerListItem creating item for: [%d](%s)"), InPlayerIndex, *InPlayerName);

    if (PlayerListItemClass)
    {
        UPlayerListItem* NewItem = CreateWidget<UPlayerListItem>(this, PlayerListItemClass);
        if (NewItem)
        {
            NewItem->SetPlayerStatus(InPlayerIndex, LocalPlayerIndex, InPlayerName);
            return NewItem;
        }
    }
    return nullptr;
}

void UPlayerWidget::OnPlayerListUpdated(const TArray<FString>& NewPlayerList)
{
    UpdatePlayerList(NewPlayerList);
}