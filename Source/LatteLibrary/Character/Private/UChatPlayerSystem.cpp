// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UChatPlayerSystem.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

#include "APlayerControl.h"

#include "YiSan/UI/Public/UChatBoxWidget.h"
#include "YiSan/UI/Public/UChatUIWidget.h"

UChatPlayerSystem::UChatPlayerSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    SetIsReplicatedByDefault(true);
}

void UChatPlayerSystem::InitSytstem(APlayerControl* PC)
{
    if (PC->IsLocalController())
    {
        ChatUIRef = CreateWidget<UChatUIWidget>(PC, ChatUIClass);
        if (ChatUIRef)
        {
            ChatUIRef->AddToViewport(0);
            ChatBoxRef = ChatUIRef->WBP_ChatBox;
        }
    }

    // 입력 바인딩
    if (PC->InputComponent)
    {
        PC->InputComponent->BindAction("Enter", IE_Pressed, this, &UChatPlayerSystem::OnEnterPressed);
        PC->InputComponent->BindAction("MouseWheelUp", IE_Pressed, this, &UChatPlayerSystem::OnMouseWheelUp);
        PC->InputComponent->BindAction("MouseWheelDown", IE_Pressed, this, &UChatPlayerSystem::OnMouseWheelDown);
    }
}

void UChatPlayerSystem::OnEnterPressed()
{
    ChatBoxRef->FocusChat();
}

void UChatPlayerSystem::OnMouseWheelUp()
{
    // 위로
    ChatBoxRef->Scroll(true);
}

void UChatPlayerSystem::OnMouseWheelDown()
{
    // 아래로
    ChatBoxRef->Scroll(false);
}

void UChatPlayerSystem::SendChatMessage_Implementation(const FString& Message)
{
    if (Message.IsEmpty())
        return;

    AddChatMessageOnAllClients(Message);
}

void UChatPlayerSystem::AddChatMessageOnAllClients_Implementation(const FString& Message)
{
    for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
    {
        if (APlayerController* PC = Cast<APlayerController>(PS->GetOwner()))
        {
            if (UChatPlayerSystem* Comp = PC->FindComponentByClass<UChatPlayerSystem>())
            {
                Comp->AddChatMessageOnOwningClient(Message);
            }
        }
    }
}

void UChatPlayerSystem::AddChatMessageOnOwningClient_Implementation(const FString& Message)
{
    ChatBoxRef->AddChatMessage(Message);
}