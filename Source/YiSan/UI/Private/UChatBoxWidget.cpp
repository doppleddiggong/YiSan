// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UChatBoxWidget.h"

#include "GameLogging.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "UChatPlayerSystem.h"
#include "UChatEntryWidget.h"

void UChatBoxWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (auto PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        ChatPlayerSystem = Cast<UChatPlayerSystem>(PC->GetComponentByClass(UChatPlayerSystem::StaticClass()));

    ChatInput->OnTextCommitted.AddDynamic(this, &UChatBoxWidget::OnTextCommittedHandler);
}

void UChatBoxWidget::OnTextCommittedHandler(const FText& Text, ETextCommit::Type CommitMethod)
{
    PRINTLOG(TEXT("OnTextCommitted()"));

    if (CommitMethod != ETextCommit::OnEnter)
        return;

    const FString InputString = Text.ToString();

    if (InputString.IsEmpty())
    {
        PRINTLOG(TEXT("InputString.IsEmpty()"));
        ExitChat();
        return;
    }

    if (!ChatPlayerSystem)
        return;

    const FString Message = FString::Printf(TEXT("%s : %s"), *GetPlayerDisplayName(), *InputString);

    ChatPlayerSystem->ServerRPC_SendChatMessage(Message);
    ChatInput->SetText(FText::GetEmpty());
    PRINTLOG(TEXT("ChatInput->SetText(FText::GetEmpty())"));
    ExitChat();
}

void UChatBoxWidget::FocusChat()
{
    PRINTLOG(TEXT("FocusChat()"));

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        ChatInput->SetIsEnabled(true);
        ChatInput->SetUserFocus(PC);

        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(ChatInput->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;

        bChatFocused = true;
    }
}

void UChatBoxWidget::ExitChat()
{
    PRINTLOG(TEXT("ExitChat()"));

    bChatFocused = false;

    ChatInput->SetText(FText::GetEmpty());
    ChatInput->SetIsEnabled(false);

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}

void UChatBoxWidget::Scroll(bool bUp)
{
    const float CurrentOffset = ScrollBox->GetScrollOffset();
    const float Direction = bUp ? -1.0f : 1.0f;
    float NewOffset = CurrentOffset + (ScrollMultiplier * Direction);

    const float MaxOffset = ScrollBox->GetScrollOffsetOfEnd();
    NewOffset = FMath::Clamp(NewOffset, 0.0f, MaxOffset);

    ScrollBox->SetScrollOffset(NewOffset);
}

void UChatBoxWidget::AddChatMessage(const FString& Message)
{
    if (!ScrollBox || !ChatEntryClass)
        return;

    UChatEntryWidget* NewEntry = CreateWidget<UChatEntryWidget>(this, ChatEntryClass);
    NewEntry->Message = Message;
    ScrollBox->AddChild(NewEntry);
    ScrollBox->ScrollToEnd();
}

FString UChatBoxWidget::GetPlayerDisplayName() const
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APlayerState* PS = PC->PlayerState)
        {
            return PS->GetPlayerName();
        }
    }
    return TEXT("Unknown");
}