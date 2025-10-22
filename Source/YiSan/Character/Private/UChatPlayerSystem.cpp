// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UChatPlayerSystem.h"

#include "UChatBoxWidget.h"
#include "APlayerActor.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"


UChatPlayerSystem::UChatPlayerSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UChatPlayerSystem::InitSystem(
	APlayerActor* InOwner,
	UChatBoxWidget* InChatBox)
{
	this->Owner = InOwner;

	this->ChatBoxWidget = InChatBox;
	this->ChatBoxWidget->InitSystem( Owner );
}

void UChatPlayerSystem::OnEnterPressed()
{
	ChatBoxWidget->FocusChat();
}

void UChatPlayerSystem::OnScrollUp()
{
	ChatBoxWidget->Scroll(true);
}

void UChatPlayerSystem::OnScrollDown()
{
	ChatBoxWidget->Scroll(false);
}

void UChatPlayerSystem::ServerRPC_SendChatMessage_Implementation(const FString& Message)
{ 
	if (Message.IsEmpty())
		return;

	MulticastRPC_AddChatMessage(Message);
}

void UChatPlayerSystem::MulticastRPC_AddChatMessage_Implementation(const FString& Message)
{
	ChatBoxWidget->AddChatMessage(Message);
}

void UChatPlayerSystem::ClientRPC_AddChatMessage_Implementation (const FString& Message)
{
	ChatBoxWidget->AddChatMessage(Message);
}
