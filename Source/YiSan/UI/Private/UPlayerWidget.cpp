// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UPlayerWidget.h"
#include "UNetworkGameInstanceSubsystem.h" // Added include
#include "Engine/GameInstance.h" // Added include for GameInstance
#include "Components/TextBlock.h" // Added include for UTextBlock
#include "Components/VerticalBox.h" // Added include for UVerticalBox
#include "Blueprint/WidgetTree.h" // Added include for WidgetTree
#include "Engine/Font.h" // Added include for UFont
#include "GameLogging.h" // Assuming this is for PRINTLOG

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
    if (!PlayerListContainer) return;

    PlayerListContainer->ClearChildren(); // Clear existing entries
    PlayerListContainer->SetVisibility(ESlateVisibility::Visible);

    for (const FString& PlayerInfoString : playerNames)
    {
        TArray<FString> PlayerInfo;
        PlayerInfoString.ParseIntoArray(PlayerInfo, TEXT(":"), true);

        if (PlayerInfo.Num() == 5)
        {
            FString PlayerName = PlayerInfo[0];
            bool bIsHost = PlayerInfo[1].ToBool();
            bool bIsReady = PlayerInfo[2].ToBool();
            int32 PlayerIndex = FCString::Atoi(*PlayerInfo[3]);
            bool bIsLocalPlayer = PlayerInfo[4].ToBool();

            UPlayerListItem* PlayerListItem = CreatePlayerListItem(PlayerName);
            if (PlayerListItem)
            {
                PlayerListItem->SetPlayerStatus(bIsHost, bIsLocalPlayer, PlayerIndex);
                PlayerListContainer->AddChild(PlayerListItem);
            }
        }
    }
}

UPlayerListItem* UPlayerWidget::CreatePlayerListItem(const FString& playerName)
{
    PRINTLOG(TEXT("UPlayerWidget::CreatePlayerListItem creating item for: %s"), *playerName);

    if (PlayerListItemClass)
    {
        UPlayerListItem* NewItem = CreateWidget<UPlayerListItem>(this, PlayerListItemClass);
        if (NewItem)
        {
            NewItem->SetPlayerName(playerName);
            return NewItem;
        }
    }
    return nullptr;
}

void UPlayerWidget::OnPlayerListUpdated(const TArray<FString>& NewPlayerList)
{
    UpdatePlayerList(NewPlayerList);
}