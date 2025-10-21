// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UChatPlayerSystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "YiSan/UI/Public/UChatBoxWidget.h"

UChatPlayerSystem::UChatPlayerSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UChatPlayerSystem::InitSystem(UChatBoxWidget* InChatBox)
{
	ChatBoxRef = InChatBox;
}

void UChatPlayerSystem::OnEnterPressed()
{
	if (ChatBoxRef)
		ChatBoxRef->FocusChat();
}

void UChatPlayerSystem::OnScrollUp()
{
	if (ChatBoxRef)
		ChatBoxRef->Scroll(true);
}

void UChatPlayerSystem::OnScrollDown()
{
	if (ChatBoxRef)
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
			if (UChatPlayerSystem* Comp = PC->GetPawn()->FindComponentByClass<UChatPlayerSystem>())
			{
				Comp->AddChatMessageOnOwningClient(Message);
			}
		}
	}
}

void UChatPlayerSystem::AddChatMessageOnOwningClient_Implementation(const FString& Message)
{
	if (ChatBoxRef)
		ChatBoxRef->AddChatMessage(Message);
}
