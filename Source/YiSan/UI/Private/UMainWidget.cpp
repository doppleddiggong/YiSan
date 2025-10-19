// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UMainWidget.h"
#include "GameLogging.h"
#include "UBroadcastManager.h"
#include "UHttpNetworkSystem.h"
#include "APlayerActor.h"
#include "FGPTContext.h"
#include "UVoiceFunctionLibrary.h"

#include "USmallPopup.h"
#include "UMegaPopup.h"

#include "Components/CanvasPanel.h"
#include "Components/EditableTextBox.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWaveProcedural.h"
#include "Input/Reply.h"

void UMainWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (InputText)
        InputText->OnTextCommitted.AddDynamic(this, &UMainWidget::OnMessageComitted);

    ChatBox->SetVisibility(ESlateVisibility::Hidden);
    
    if ( APlayerController* PC = GetWorld()->GetFirstPlayerController() )
    {
        FInputModeGameOnly InputMode;
        InputMode.SetConsumeCaptureMouseDown(false);
        
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
    
    BroadcastManager = UBroadcastManager::Get(GetWorld());
    if (BroadcastManager)
    {
        BroadcastManager->OnNearBuilding.AddDynamic(this, &UMainWidget::OnNearBuilding);
		BroadcastManager->OnMegaPopupClosed.AddDynamic(this, &UMainWidget::OnMegaPopupClosed);
    }

	// Popup 초기 상태 설정 (공간 차지하지 않게 collapsed 로 성정)
    if (SmallPopupCtn)
        SmallPopupCtn->SetVisibility(ESlateVisibility::Collapsed);
	
    if (MegaPopupCtn)
        MegaPopupCtn->SetVisibility(ESlateVisibility::Collapsed);
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

bool UMainWidget::IsMegaPopupVisible()
{
	if ( MegaPopupCtn == nullptr )
		return false;
	return MegaPopupCtn->GetVisibility() == ESlateVisibility::Visible;
}

bool UMainWidget::IsSmallPopupVisible()
{
	if ( SmallPopupCtn == nullptr )
		return false;
    return SmallPopupCtn->GetVisibility() == ESlateVisibility::Visible;
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

void UMainWidget::ToggleMegaPopup()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return;

	if (IsMegaPopupVisible())
	{
		// megapopup 및 smallpopup 숨기자
		MegaPopupCtn->OnClose();
		MegaPopupCtn->SetVisibility(ESlateVisibility::Collapsed);
		SmallPopupCtn->SetVisibility(ESlateVisibility::Collapsed);

		// 입력 모드도 원래로 돌린다
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);

		// 플레이어 컨트롤 Enable 한다
		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->EnableInput(PC);
		}

		SmallPopupCtn->UpdateBuildingInfo(CurNearBuildingType);
	}
	else if (IsSmallPopupVisible())
	{
		// mega popup 표시
		MegaPopupCtn->SetVisibility(ESlateVisibility::Visible);
		SmallPopupCtn->SetVisibility(ESlateVisibility::Collapsed);

		// 입력모드 전환 (player input 전화)
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(MegaPopupCtn->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);

		// 플레이어 컨트롤 disable 하자
		if (APawn* Pawn = PC->GetPawn())
			Pawn->DisableInput(PC);

		MegaPopupCtn->UpdateBuildingInfo(CurNearBuildingType);
	}
}

void UMainWidget::OnMegaPopupClosed()
{
	ToggleMegaPopup();
}

void UMainWidget::OnNearBuilding(EBuildingType InBuildingType)
{
	if ( CurNearBuildingType == InBuildingType )
		return;

	if (IsMegaPopupVisible())
		return;

	CurNearBuildingType = InBuildingType;

    if (InBuildingType != EBuildingType::None)
    {
        SmallPopupCtn->SetVisibility(ESlateVisibility::Visible);
    	SmallPopupCtn->UpdateBuildingInfo(InBuildingType);
    }
    else
    {
        SmallPopupCtn->SetVisibility(ESlateVisibility::Collapsed);
    }
}
//
// // 팝업 눌렀을때 닫히게 하고싶다
// FReply UMainWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
// {
// 	if ( IsMegaPopupVisible() )
// 	{
// 		ToggleMegaPopup();
// 		return FReply::Handled();
// 	}
// 	return FReply::Unhandled();
// }