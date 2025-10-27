// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

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
			// 플레이어 이름 가져오기
			FString PlayerName = Owner->GetPlayerDisplayName();

			// Dasan NPC가 질문을 받을 수 있는지 체크 (PlayerController를 통해 서버에 요청)
			if (APlayerControl* PC = Owner->GetController<APlayerControl>())
			{
				PC->ServerRPC_TryStartAnswer(PlayerName);
			}

			// 채팅 메시지 전송
		    FChatMessage ChatMessage(EChatMessageType::User, PlayerName, *InputString);
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
			FChatMessage ChatMessage(EChatMessageType::NPC, GameString::NPC,Response.gpt_response_text);
			ChatPlayerSystem->ServerRPC_SendChatMessage(ChatMessage);

			Owner->PlayTTSAudio(Response.audio_data);
		}

		// GPT 응답 완료 후 답변 종료 처리 (PlayerController를 통해 서버에 요청)
		// TTS가 재생되지 않거나 OnTTSFinished가 호출되지 않는 경우 대비
		// (OnTTSFinished에서도 FinishAnswer가 호출되지만 중복 호출은 안전함)
		if (APlayerControl* PC = Owner->GetController<APlayerControl>())
		{
			PC->ServerRPC_FinishAnswer();
			PRINTLOG(TEXT("[ChatBox] GPT 응답 완료 - FinishAnswer 호출"));
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

	// //현재 스크롤 위치
	// float scrollOffset = ScrollBox->GetScrollOffset();
	// // 스크롤 맨 끝일때 값
	// float scrollOffsetOfEnd = ScrollBox->GetScrollOffsetOfEnd();

    UChatEntryWidget* NewEntry = CreateWidget<UChatEntryWidget>(this, ChatEntryClass);
    NewEntry->ChatMessageData = ChatMessage;
    ScrollBox->AddChild(NewEntry);
    // ScrollBox->ScrollToEnd();

	// 만약에 스크롤이 위치가 맽 끝이라면
	// if( scrollOffset == scrollOffsetOfEnd )
	{
		// 개행되는 채팅이 추가되면 한줄로 크기를 인식해서 발생하는 문제 때문에
		// ScrollToEnd 0.01초 뒤에 실행
		FTimerHandle timerHandle;
		GetWorld()->GetTimerManager().SetTimer(timerHandle, [this]()
		{
		   // 스크롤 위치를 맨 끝으로 해라!
		   ScrollBox->ScrollToEnd();
		},0.1f, false);
	}
}
