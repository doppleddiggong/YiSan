// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UChatBoxWidget.cpp
 * @brief UChatBoxWidget의 동작을 구현합니다.
 */
#include "UChatBoxWidget.h"

#include "APlayerActor.h"
#include "APlayerControl.h"
#include "GameLogging.h"
#include "UBroadcastManager.h"
#include "UChatPlayerSystem.h"
#include "UChatEntryWidget.h"
#include "UHttpNetworkSystem.h"
#include "UVoiceFunctionLibrary.h"
#include "ADasanActor.h"
#include "UAnswerStateSystem.h"
#include "AYisanGameState.h"

#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "YiSan/YiSan.h"


void UChatBoxWidget::NativeConstruct()
{
    Super::NativeConstruct();
    ChatInput->OnTextCommitted.AddDynamic(this, &UChatBoxWidget::OnTextCommittedHandler);
}

void UChatBoxWidget::InitSystem(APlayerActor* InOwner)
{
    this->Owner = InOwner;
    this->ChatPlayerSystem = Owner->ChatPlayerSystem;

    BroadcastManager = UBroadcastManager::Get(GetWorld());
}

void UChatBoxWidget::OnTextCommittedHandler(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter)
		return;

	const FString InputString = Text.ToString();

	if (!InputString.IsEmpty())
	{
		if (ChatPlayerSystem)
		{
			int32 PlayerIndex = Owner->GetPlayerIndex();
			FString PlayerName = Owner->GetPlayerDisplayName();

			// 채팅 메시지 전송
		    FChatMessage ChatMessage(EChatMessageType::User, PlayerIndex, PlayerName, *InputString);
		    ChatPlayerSystem->ServerRPC_SendChatMessage(ChatMessage);

			// GPT 질문 요청
		    this->Ask(InputString, Owner->GetGPTContext());
		}
	}

	ExitChat();
}

void UChatBoxWidget::Ask(const FString& InMsg, const FGPTContext& SpatialContext)
{
	// 로컬 플레이어만 GPT 요청
	if (!IsValid(Owner) || !Owner->IsLocallyControlled())
	{
		return;
	}

	if (auto ReqNetwork = UHttpNetworkSystem::Get(GetWorld()))
		ReqNetwork->RequestGPT(InMsg, SpatialContext, FResponseAskDelegate::CreateUObject(this, &UChatBoxWidget::OnResponseAsk));
}

void UChatBoxWidget::OnResponseAsk(FResponseAsk& Response, bool bSuccess)
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
			BroadcastManager->SendExecVoiceCommand( VoiceCommand, Owner );
		}
		else
		{
			FChatMessage ChatMessage(EChatMessageType::NPC, -1, GameString::NPC, Response.gpt_response_text);
			ChatPlayerSystem->ServerRPC_SendChatMessage(ChatMessage);
		}
	}
	else
	{
		PRINTLOG( TEXT("--- Network Response Received (FAIL) ---"));

		// 실패한 경우에도 답변 종료 처리 (PlayerController를 통해 서버에 요청)
		if (APlayerControl* PC = Owner->GetController<APlayerControl>())
		{
			PC->ServerRPC_FinishAnswer();
			PRINTLOG(TEXT("[ChatBox] GPT 응답 실패 - FinishAnswer 호출"));
		}
	}
}


void UChatBoxWidget::FocusChat()
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        ChatInput->SetIsEnabled(true);
        ChatInput->SetUserFocus(PC);

        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(ChatInput->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;

        bChatFocused = true;
    }
}

void UChatBoxWidget::ExitChat()
{
    bChatFocused = false;

    ChatInput->SetText(FText::GetEmpty());
    ChatInput->SetIsEnabled(false);

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        FInputModeGameOnly InputMode;
        InputMode.SetConsumeCaptureMouseDown(false);
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

void UChatBoxWidget::AddChatMessage(const FChatMessage& ChatMessage)
{
    if (!ScrollBox || !ChatEntryClass)
        return;

    UChatEntryWidget* NewEntry = CreateWidget<UChatEntryWidget>(this, ChatEntryClass);
    NewEntry->ChatMessageData = ChatMessage;

	ChatMessagesBox->AddChildToVerticalBox(NewEntry);

	// 레이아웃 갱신 유도 (줄바꿈 등 레이아웃 반영)
	NewEntry->InvalidateLayoutAndVolatility();
	ChatMessagesBox->InvalidateLayoutAndVolatility();
	ScrollBox->InvalidateLayoutAndVolatility();
	
	// 0.01초 딜레이 → 확실히 Slate Layout Pass 이후 실행
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this]()
	{
		ScrollBox->ScrollToEnd();
	}, 0.1f, false);
}
