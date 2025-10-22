// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UChatPlayerSystem.h"
#include "UChatBoxWidget.h"
#include "APlayerActor.h"

#include "GameFramework/PlayerController.h"

UChatPlayerSystem::UChatPlayerSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UChatPlayerSystem::InitSystem(UChatBoxWidget* InChatBox)
{
	this->Owner = Cast<APlayerActor>(GetOwner());
	this->ChatBoxWidget = InChatBox;

	if (IsValid(ChatBoxWidget) && IsValid(Owner))
	{
		ChatBoxWidget->InitSystem(Owner);
	}
}

void UChatPlayerSystem::OnEnterPressed()
{
	if (!IsValid(ChatBoxWidget))
		return;

	ChatBoxWidget->FocusChat();
}

void UChatPlayerSystem::OnScrollUp()
{
	if (!IsValid(ChatBoxWidget))
		return;
	
	ChatBoxWidget->Scroll(true);
}

void UChatPlayerSystem::OnScrollDown()
{
	if (!IsValid(ChatBoxWidget))
		return;

	ChatBoxWidget->Scroll(false);
}

void UChatPlayerSystem::ServerRPC_SendChatMessage_Implementation(const FChatMessage& ChatMessage)
{
	if (ChatMessage.Message.IsEmpty())
		return;

	MulticastRPC_AddChatMessage(ChatMessage);
}

void UChatPlayerSystem::MulticastRPC_AddChatMessage_Implementation(const FChatMessage& ChatMessage)
{
	// 각 클라이언트의 로컬 플레이어 찾아서 UI 업데이트
	if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
	{
		if (APlayerActor* LocalPlayer = Cast<APlayerActor>(LocalPC->GetPawn()))
		{
			if (IsValid(LocalPlayer->ChatBoxWidget))
			{
				LocalPlayer->ChatBoxWidget->AddChatMessage(ChatMessage);
			}
		}
	}
}

void UChatPlayerSystem::ClientRPC_AddChatMessage_Implementation(const FChatMessage& ChatMessage)
{
	// 클라이언트의 로컬 플레이어 찾아서 UI 업데이트
	if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
	{
		if (APlayerActor* LocalPlayer = Cast<APlayerActor>(LocalPC->GetPawn()))
		{
			if (IsValid(LocalPlayer->ChatBoxWidget))
			{
				LocalPlayer->ChatBoxWidget->AddChatMessage(ChatMessage);
			}
		}
	}
}