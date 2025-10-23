// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UChatBoxWidget.h"

#include "APlayerActor.h"
#include "GameLogging.h"
#include "UBroadcastManager.h"
#include "UChatPlayerSystem.h"
#include "UChatEntryWidget.h"
#include "UHttpNetworkSystem.h"
#include "UVoiceFunctionLibrary.h"

#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Sound/SoundWaveProcedural.h"
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
		    FChatMessage ChatMessage(EChatMessageType::User, Owner->GetPlayerDisplayName(), *InputString);
		    ChatPlayerSystem->ServerRPC_SendChatMessage(ChatMessage);

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
		PRINTLOG(TEXT("SendChatMessage return | !IsValid(Owner) || !Owner->IsLocallyControlled() "));
		return;
	}

	if (auto ReqNetwork = UHttpNetworkSystem::Get(GetWorld()))
	{
		ReqNetwork->RequestGPT(InMsg, SpatialContext, FResponseAskDelegate::CreateUObject(this, &UChatBoxWidget::OnResponseAsk));
	}
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
			BroadcastManager->SendExecVoiceCommand( VoiceCommand );
		}
		else
		{
			FChatMessage ChatMessage(EChatMessageType::NPC, GameString::NPC,Response.gpt_response_text);
			ChatPlayerSystem->ServerRPC_SendChatMessage(ChatMessage);

			auto SoundWave = UVoiceFunctionLibrary::CreateProceduralSoundWaveFromWavData(Response.audio_data);
			if ( IsValid(SoundWave))
				UGameplayStatics::PlaySound2D(this, SoundWave);
		}
	}
	else
	{
		PRINTLOG( TEXT("--- Network Response Received (FAIL) ---"));
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
    ScrollBox->AddChild(NewEntry);
    ScrollBox->ScrollToEnd();
}