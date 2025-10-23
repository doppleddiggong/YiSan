// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UChatPlayerSystem.h"
#include "UChatBoxWidget.h"
#include "APlayerActor.h"
#include "GameLogging.h"
#include "UBroadcastManager.h"
#include "UHttpNetworkSystem.h"
#include "UVoiceFunctionLibrary.h"

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

void UChatPlayerSystem::OnEnter()
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

void UChatPlayerSystem::Ask(const FString& InMsg, const FGPTContext& SpatialContext)
{
	// 로컬 플레이어만 GPT 요청
	if (!IsValid(Owner) || !Owner->IsLocallyControlled())
	{
		PRINTLOG(TEXT("SendChatMessage return | !IsValid(Owner) || !Owner->IsLocallyControlled() "));
		return;
	}



	
	if (auto ReqNetwork = UHttpNetworkSystem::Get(GetWorld()))
	{
		ReqNetwork->RequestGPT(InMsg, SpatialContext, FResponseAskDelegate::CreateUObject(this, &UChatPlayerSystem::OnResponseAsk));
	}
}

void UChatPlayerSystem::OnResponseAsk(FResponseAsk& Response, bool bSuccess)
{
	if (bSuccess)
	{
		PRINTLOG(TEXT("OnResponseAsk: Received transcribed_text : %s"), *Response.transcribed_text);
		PRINTLOG(TEXT("OnResponseAsk: Received gpt_response_text : %s"), *Response.gpt_response_text);
		PRINTLOG(TEXT("OnResponseAsk: Received audio data size: %d"), Response.audio_data.Num());

		auto VoiceCommand = UVoiceFunctionLibrary::GetVoiceCommand(Response.gpt_response_text);
		PRINTLOG(TEXT("OnResponseAsk: VoiceCommand result is %d"), static_cast<int32>(VoiceCommand));
		if ( VoiceCommand != EVoiceCommandType::None )
		{
			if ( auto BroadcastManager = UBroadcastManager::Get(GetWorld()) )
			{
				BroadcastManager->SendExecVoiceCommand( VoiceCommand );
			}
		}
		else
		{
			FChatMessage ChatMessage(EChatMessageType::NPC,
				TEXT("정약용"),
				Response.gpt_response_text,
				Response.audio_data );
			this->ServerRPC_SendChatMessage(ChatMessage);
		}
	}
	else
	{
		PRINTLOG( TEXT("--- Network Response Received (FAIL) ---"));
	}
}


void UChatPlayerSystem::ServerRPC_SendChatMessage_Implementation(const FChatMessage& ChatMessage)
{
	if (ChatMessage.Message.IsEmpty())
		return;

	MulticastRPC_AddChatMessage(ChatMessage);
}

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