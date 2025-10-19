// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UMainWidget.h"
#include "GameLogging.h"
#include "UBroadcastManager.h"
#include "UHttpNetworkSystem.h"
#include "APlayerActor.h"
#include "FGPTContext.h"
#include "UVoiceFunctionLibrary.h"

//addUobject 안되길레 추가
#include "Delegates/Delegate.h"

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
    
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        FInputModeGameOnly InputMode;
        InputMode.SetConsumeCaptureMouseDown(false);
        
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }

    
    BroadcastManager = UBroadcastManager::Get(GetWorld());
    if (BroadcastManager)
    {
        // OnNearBuilding 델리게이트에 
        BroadcastManager->OnNearBuilding.AddDynamic(this, &UMainWidget::OnNearBuildingBroadcast);
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
	// 메가 팝업이 열려있지 않고, 스몰 팝업이 보이지 않는다면 아무것도 하지 않는다.
	if (!bIsMegaPopupVisible && SmallPopupCtn && SmallPopupCtn->GetVisibility() != ESlateVisibility::Visible)return;
	if (!MegaPopupCtn)return;
	// player controller 로 신호주자
	APlayerController* PC = GetOwningPlayer();
	if (!PC)return;

	bIsMegaPopupVisible = !bIsMegaPopupVisible;

	if (bIsMegaPopupVisible)
	{
		// mega popup 표시
		MegaPopupCtn->SetVisibility(ESlateVisibility::Visible);
		if(SmallPopupCtn)
		{
			SmallPopupCtn->SetVisibility(ESlateVisibility::Collapsed);
		}

		// 입력모드 전환 (player input 전화)
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(MegaPopupCtn->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);

		// 플레이어 컨트롤 disable 하자
		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->DisableInput(PC);
		}

		if (CurNearBuildingType != EBuildingType::None)
		{
			// enum 의 건물 가져오기
			FString BuildingName = StaticEnum<EBuildingType>()->GetDisplayNameTextByValue((int32)CurNearBuildingType).ToString();
			// enum 값 확인용
			FString Prompt = FString::Printf(TEXT("%s에 대해 자세히 설명해줘."), *BuildingName);
			PRINTLOG(TEXT("현재 건물은:%s"), *BuildingName);
			
			FGPTContext SpatialContext = BuildSpatialContext();
			// 생성해서 함수에 response한다
			if (auto ReqNetwork = UHttpNetworkSystem::Get(GetWorld()))
			{
				ReqNetwork->RequestGPT(Prompt, SpatialContext, 
					FResponseAskDelegate::CreateUObject(this, &UMainWidget::OnMegaPopupResponse));
			}
		}

		// 시네마틱 뷰 활성화 할떄 사용할꺼
		// PC->SetCinematicMode(true, false, false, true, true);

		// 블루프린트 이벤트 호출 (상세 정보 업데이트) 
		BPI_UpdateDetailedInfo(CurNearBuildingType);
	}
	else
	{
		// megapopup 및 smallpopup 숨기자
		MegaPopupCtn->SetVisibility(ESlateVisibility::Collapsed);
		if (SmallPopupCtn)
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

		// 시네마틱 뷰 비활성화 하고싶을때
		// PC->SetCinematicMode(false, false, false, true, true);
	}
}

void UMainWidget::OnNearBuildingBroadcast(EBuildingType BuildingType)
{
	// 주백이형 말씀대로  타입 이 같다면 리턴한다
	if (BuildingType==CurNearBuildingType) return;
	CurNearBuildingType = BuildingType;

	// location이 바뀐다면 small popup 의 속성이 변하지 않게 하기 위한 안전 트리거
	// megapopup 이 켜졌다면 상태를 변경하지않고 리턴한다 
	if (bIsMegaPopupVisible)return;
	if (!SmallPopupCtn)return;
	
    if (!SmallPopupCtn)
        return;

    // 타입이 none이 아니라면 표시한다
    if (BuildingType != EBuildingType::None)
    {
        SmallPopupCtn->SetVisibility(ESlateVisibility::Visible);
        
        // 블루프린트 이벤트 호출
        BPI_UpdateSmallPopupText(BuildingType);
    }
    else
    {
        SmallPopupCtn->SetVisibility(ESlateVisibility::Collapsed);
    }
}

// gpt 의 대답을 처리할 함수를 생성하자
void UMainWidget::OnMegaPopupResponse(FResponseAsk& Response, bool bSuccess)
{
	//megapopup 이 아니라면 리턴하자
	if (!MegaPopupCtn)
		return;
	// 값이 들어왔다면
	if (bSuccess)
	{
		PRINTLOG(TEXT("수신된 대답은: %s"),*Response.gpt_response_text);
        
		// GPT 응답 텍스트설정
		MegaPopupCtn->SetDescription(Response.gpt_response_text);
	}
	// 만약 실패했다면
	else
	{
		PRINTLOG(TEXT("GPT 에게 응답이 됮 않았습니다"));
		MegaPopupCtn->SetDescription(TEXT("설명을 불러오는데 실패했습니다."));
	}
}

// 팝업 눌렀을때 닫히게 하고싶다
FReply UMainWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsMegaPopupVisible)
	{
		ToggleMegaPopup();
		return FReply::Handled();
	}
	return FReply::Unhandled();
}