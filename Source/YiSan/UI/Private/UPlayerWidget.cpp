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
            NetworkSubsystem->RequestPlayerListRefresh(); // Request refresh via subsystem
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

    for (const FString& Name : playerNames)
    {
        UTextBlock* PlayerText = CreatePlayerText(Name);
        if (PlayerText)
        {
            PlayerListContainer->AddChild(PlayerText);
        }
    }
}

UTextBlock* UPlayerWidget::CreatePlayerText(const FString& playerName)
{
    PRINTLOG(TEXT("UPlayerWidget::CreatePlayerText creating text for: %s"), *playerName);
    UTextBlock* newText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    if (newText)
    {
        newText->SetText(FText::FromString(playerName));
        newText->SetJustification(ETextJustify::Right);
        newText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

        // 폰트 적용 (Assuming the same font as StartUI)
        UFont* fontObj = LoadObject<UFont>(nullptr, TEXT("/Game/CustomContents/UI/Fonts/NotoSerifKR-Regular_Font.NotoSerifKR-Regular_Font.NotoSerifKR-Regular_Font"));
        if (fontObj)
        {
            FSlateFontInfo fontInfo;
            fontInfo.FontObject = fontObj;
            fontInfo.Size = 20; // 원하는 크기
            newText->SetFont(fontInfo);
        }
    }
    return newText;
}

void UPlayerWidget::OnPlayerListUpdated(const TArray<FString>& NewPlayerList)
{
    UpdatePlayerList(NewPlayerList);
}