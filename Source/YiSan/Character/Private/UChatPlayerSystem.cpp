// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UChatPlayerSystem.h"
#include "UChatBoxWidget.h"
#include "APlayerActor.h"
#include "AYisanGameState.h"
#include "UGameSoundManager.h"
#include "YiSan/YiSan.h"
#include "GameFramework/PlayerController.h"

/**
 * @file UChatPlayerSystem.cpp
 * @brief UChatPlayerSystem의 동작을 구현합니다.
 */

/** @brief 복제를 활성화하고 Tick을 비활성화하는 기본 생성자입니다. */
UChatPlayerSystem::UChatPlayerSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

/**
 * @brief 소유 액터와 채팅 위젯 인스턴스로 채팅 시스템을 초기화합니다.
 * @param InChatBox 채팅 기록을 표시하고 입력을 받는 위젯입니다.
 */
void UChatPlayerSystem::InitSystem(UChatBoxWidget* InChatBox)
{
	this->Owner = Cast<APlayerActor>(GetOwner());
	this->ChatBoxWidget = InChatBox;

	if (IsValid(ChatBoxWidget) && IsValid(Owner))
	{
		ChatBoxWidget->InitSystem(Owner);
	}
}

/** @brief 엔터 키 입력을 처리하여 채팅 박스에 포커스를 맞춥니다. */
void UChatPlayerSystem::OnEnter()
{
	if (!IsValid(ChatBoxWidget))
		return;

	ChatBoxWidget->FocusChat();
}

/** @brief 채팅 UI에 위로 스크롤하도록 요청합니다. */
void UChatPlayerSystem::OnScrollUp()
{
	if (!IsValid(ChatBoxWidget))
		return;
	
	ChatBoxWidget->Scroll(true);
}

/** @brief 채팅 UI에 아래로 스크롤하도록 요청합니다. */
void UChatPlayerSystem::OnScrollDown()
{
	if (!IsValid(ChatBoxWidget))
		return;

	ChatBoxWidget->Scroll(false);
}

/** @brief 채팅 메시지를 검증하고 서버에서 모든 클라이언트로 전달합니다. */
void UChatPlayerSystem::ServerRPC_SendChatMessage_Implementation(const FChatMessage& ChatMessage)
{
	if (ChatMessage.Message.IsEmpty())
		return;

	MulticastRPC_AddChatMessage(ChatMessage);
}

/** @brief 복제된 채팅 메시지를 수신해 로컬 UI에 반영합니다. */
void UChatPlayerSystem::MulticastRPC_AddChatMessage_Implementation(const FChatMessage& ChatMessage)
{
	// 현재 클라이언트의 로컬 플레이어 컨트롤러를 가져옴 (멀티플레이 환경에서도 각 클라에 1개만 존재)
	// 없으면 UI 접근 불가 → 조기 종료
	auto* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) 
		return;

	// 로컬 컨트롤러가 조종 중인 Pawn을 APlayerActor 타입으로 변환
	// 변환 실패 시 종료 (월드 내 Pawn이 없거나 아직 스폰 전일 수 있음)
	auto* Player = Cast<APlayerActor>(PC->GetPawn());
	if (!Player) 
		return;

	// 플레이어가 보유한 채팅 UI 위젯 참조
	// UI 위젯이 생성되지 않았으면 종료
	auto* ChatBox = Player->ChatBoxWidget.Get();
	if (!ChatBox) 
		return;

	// 로컬 클라이언트의 채팅창에 서버에서 전달받은 메시지를 출력
	ChatBox->AddChatMessage(ChatMessage);
}

/** @brief 소유 플레이어 입장을 알리는 지역화 메시지를 브로드캐스트합니다. */
void UChatPlayerSystem::AnnouncePlayerJoin()
{
	if (!IsValid(Owner) || !IsValid(ChatBoxWidget))
		return;

	const FString Message = FString::Printf(TEXT("%s 전하 어가, 행궁에 이르셨습니다."), *Owner->GetPlayerDisplayName());
	const FChatMessage ChatMessage(EChatMessageType::System, -1, GameString::System, Message);
	ServerRPC_SendChatMessage(ChatMessage);

	if (auto GS = GetWorld()->GetGameState<AYisanGameState>())
	{
		GS->ServerRPC_PlaySound(EGameSoundType::Enter_Game);
	}
}