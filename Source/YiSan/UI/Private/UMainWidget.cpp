// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UMainWidget.h"
#include "GameLogging.h"
#include "UBroadcastManger.h"
#include "UHttpNetworkSystem.h"
#include "APlayerActor.h"
#include "FGPTContext.h"

#include "Components/CanvasPanel.h"
#include "Components/EditableTextBox.h"

void UMainWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (InputText)
        InputText->OnTextCommitted.AddDynamic(this, &UMainWidget::OnMessageComitted);

    ChatBox->SetVisibility(ESlateVisibility::Hidden);
    
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        FInputModeGameOnly InputMode;
        InputMode.SetConsumeCaptureMouseDown(false);
        
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}

void UMainWidget::ToggleChatBox()
{
    if (ChatBox->GetVisibility() == ESlateVisibility::Hidden)
    {
        InputText->SetText(FText::GetEmpty());
        ChatBox->SetVisibility(ESlateVisibility::Visible);

        if (InputText)
            InputText->SetKeyboardFocus();

        if (APlayerController* PC = GetOwningPlayer())
        {
            FInputModeGameAndUI InputMode;
            InputMode.SetWidgetToFocus(InputText->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

            PC->SetInputMode(InputMode);
            PC->SetShowMouseCursor(true);
        }
    }
    else
    {
        SendChatMessage( InputText->GetText().ToString());
        ChatBox->SetVisibility(ESlateVisibility::Hidden);

        if (APlayerController* PC = GetOwningPlayer())
        {
            FInputModeGameOnly InputMode;
            InputMode.SetConsumeCaptureMouseDown(false);
        
            PC->SetInputMode(InputMode);
            PC->SetShowMouseCursor(false);
        }
    }
}

void UMainWidget::OnMessageComitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter)
    {
        ToggleChatBox();
    }
}

void UMainWidget::SendChatMessage(const FString& InMsg)
{
    FGPTContext SpatialContext = BuildSpatialContext();

    if (auto ReqNetwork = UHttpNetworkSystem::Get(GetWorld()))
    {
        ReqNetwork->RequestGPT(InMsg, SpatialContext, FResponseGPTDelegate::CreateUObject(this, &UMainWidget::OnResponseTestGPT));
    }
}

FGPTContext UMainWidget::BuildSpatialContext() const
{
    if (const APlayerActor* Player = Cast<APlayerActor>(GetOwningPlayerPawn()))
    {
        return Player->GetGPTContext();
    }

    return FGPTContext();
}

void UMainWidget::OnResponseTestGPT(FResponseGPT& Response, bool bSuccess)
{
    if (bSuccess)
    {
        if (auto EventManager = UBroadcastManger::Get(this))
            EventManager->SendToastMessage(Response.response);
    }
    else
    {
        PRINTLOG( TEXT("--- Network Response Received (FAIL) ---"));
    }
}

