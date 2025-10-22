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
	// // 각 클라이언트의 로컬 플레이어 찾아서 UI 업데이트
	// if (APlayerController* LocalPC = GetWorld()->GetFirstPlayerController())
	// {
	// 	if (APlayerActor* LocalPlayer = Cast<APlayerActor>(LocalPC->GetPawn()))
	// 	{
	// 		if (IsValid(LocalPlayer->ChatBoxWidget))
	// 		{
	// 			LocalPlayer->ChatBoxWidget->AddChatMessage(ChatMessage);
	// 		}
	// 	}
	// }

	// 현재 클라이언트의 로컬 플레이어 컨트롤러를 가져옴 (멀티플레이 환경에서도 각 클라에 1개만 존재)
	auto* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return; // 없으면 UI 접근 불가 → 조기 종료

	// 로컬 컨트롤러가 조종 중인 Pawn을 APlayerActor 타입으로 변환
	auto* Player = Cast<APlayerActor>(PC->GetPawn());
	if (!Player) return; // 변환 실패 시 종료 (월드 내 Pawn이 없거나 아직 스폰 전일 수 있음)

	// 플레이어가 보유한 채팅 UI 위젯 참조
	auto* ChatBox = Player->ChatBoxWidget.Get();
	if (!ChatBox) return; // UI 위젯이 생성되지 않았으면 종료

	// 로컬 클라이언트의 채팅창에 서버에서 전달받은 메시지를 출력
	ChatBox->AddChatMessage(ChatMessage);
}