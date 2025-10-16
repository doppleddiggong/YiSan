// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UMainWidget.h"
#include "GameLogging.h"
#include "UBroadcastManager.h"
#include "UHttpNetworkSystem.h"
#include "APlayerActor.h"
#include "FGPTContext.h"
#include "UVoiceFunctionLibrary.h"

#include "Components/CanvasPanel.h"
#include "Components/EditableTextBox.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWaveProcedural.h"

void UMainWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (InputText)
        InputText->OnTextCommitted.AddDynamic(this, &UMainWidget::OnMessageComitted);

    ChatBox->SetVisibility(ESlateVisibility::Hidden);
    
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        FInputModeGameOnly InputMode;
        InputMode.SetConsumeCaptureMouseDown(false);
        
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }

    BroadcastManager = UBroadcastManager::Get(GetWorld());
}

void UMainWidget::ToggleChatBox()
{
    if (ChatBox->GetVisibility() == ESlateVisibility::Hidden)
    {
        InputText->SetText(FText::GetEmpty());
        ChatBox->SetVisibility(ESlateVisibility::Visible);

        if (InputText)
            InputText->SetKeyboardFocus();

        if (APlayerController* PC = GetOwningPlayer())
        {
            FInputModeGameAndUI InputMode;
            InputMode.SetWidgetToFocus(InputText->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

            PC->SetInputMode(InputMode);
            PC->SetShowMouseCursor(true);
        }
    }
    else
    {
        SendChatMessage( InputText->GetText().ToString());
        ChatBox->SetVisibility(ESlateVisibility::Hidden);

        if (APlayerController* PC = GetOwningPlayer())
        {
            FInputModeGameOnly InputMode;
            InputMode.SetConsumeCaptureMouseDown(false);
        
            PC->SetInputMode(InputMode);
            PC->SetShowMouseCursor(false);
        }
    }
}

void UMainWidget::OnMessageComitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter)
    {
        ToggleChatBox();
    }
}

void UMainWidget::SendChatMessage(const FString& InMsg)
{
    FGPTContext SpatialContext = BuildSpatialContext();

    if (auto ReqNetwork = UHttpNetworkSystem::Get(GetWorld()))
    {
        ReqNetwork->RequestGPT(InMsg, SpatialContext, FResponseAskDelegate::CreateUObject(this, &UMainWidget::OnResponseAsk));
    }
}

FGPTContext UMainWidget::BuildSpatialContext() const
{
    if (const APlayerActor* Player = Cast<APlayerActor>(GetOwningPlayerPawn()))
    {
        return Player->GetGPTContext();
    }

    return FGPTContext();
}

void UMainWidget::OnResponseAsk(FResponseAsk& Response, bool bSuccess)
{
    if (bSuccess)
    {
        PRINTLOG(TEXT("OnResponseAsk: Received audio data size: %d"), Response.audio_data.Num());

        auto VoiceCommand = UVoiceFunctionLibrary::GetVoiceCommand(Response.gpt_response_text);
        if ( VoiceCommand != EVoiceCommandType::None )
        {
            BroadcastManager->SendExecVoiceCommand( VoiceCommand );
        }
        else
        {
            BroadcastManager->SendToastMessage(Response.gpt_response_text);

            if (Response.audio_data.Num() == 0)
            {
                PRINTLOG(TEXT("OnResponseAsk: Audio data is empty. Cannot play TTS audio."));
                return;
            }
		
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