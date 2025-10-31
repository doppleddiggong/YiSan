// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDasanWidget.h"
#include "ADasanActor.h"
#include "GameLogging.h"
#include "Macro.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

void UDasanWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기에는 설명 다이얼로그 숨김
	if (Border_Explain)
	{
		Border_Explain->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UDasanWidget::InitWidget(ADasanActor* InDasanActor)
{
	DasanActor = InDasanActor;
}

void UDasanWidget::UpdateDasanState(EDasanState MainState, ETourState TourState, EAnswerState AnswerState,
	const FString& BuildingName, const FString& PlayerName, int32 CurrentPlayers, int32 MaxPlayers)
{
	if (!Text_Msg)
	{
		PRINTLOG(TEXT("[DasanWidget] UpdateDasanState - Text_Msg is nullptr!"));
		return;
	}

	FString StateMessage = GetStateMessage(MainState, TourState, AnswerState, BuildingName, PlayerName, CurrentPlayers, MaxPlayers);
	FString ResourcePath = GetResourcePath(MainState, TourState, AnswerState);

	PRINTLOG(TEXT("[DasanWidget] UpdateDasanState - MainState: %s, TourState: %s, AnswerState: %s, Message: %s"),
		*ENUM_TO_NAME(EDasanState, MainState),
		*ENUM_TO_NAME(ETourState, TourState),
		*ENUM_TO_NAME(EAnswerState, AnswerState),
		*StateMessage);

	Text_Msg->SetText(FText::FromString(StateMessage));

	// 텍스처 로드 (올바른 경로 형식 사용)
	if (UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *ResourcePath))
	{
		Image_Icon->SetBrushFromTexture(Texture);
	}
	else
	{
		PRINTLOG(TEXT("[DasanWidget] 텍스처 로드 실패: %s"), *ResourcePath);
	}
}

void UDasanWidget::ShowExplainDialog(const FString& ExplainText)
{
	if (!Text_Explain || !Border_Explain)
	{
		PRINTLOG(TEXT("[DasanWidget] ShowExplainDialog - Text_Explain or Border_Explain is nullptr!"));
		return;
	}

	PRINTLOG(TEXT("[DasanWidget] ShowExplainDialog - Showing: %s"), *ExplainText);

	Text_Explain->SetText(FText::FromString(ExplainText));
	Border_Explain->SetVisibility(ESlateVisibility::Visible);

	// 기존 타이머 클리어
	if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(ExplainDialogTimerHandle))
		GetWorld()->GetTimerManager().ClearTimer(ExplainDialogTimerHandle);

	// 3초 후 자동 숨김
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			ExplainDialogTimerHandle,
			this,
			&UDasanWidget::HideExplainDialog,
			3.0f,
			false
		);
	}
}

void UDasanWidget::HideExplainDialog()
{
	if (!Border_Explain)
	{
		PRINTLOG(TEXT("[DasanWidget] HideExplainDialog - Border_Explain is nullptr!"));
		return;
	}

	PRINTLOG(TEXT("[DasanWidget] HideExplainDialog - Hiding dialog"));

	// Border 숨김
	Border_Explain->SetVisibility(ESlateVisibility::Collapsed);

	// 타이머 클리어
	if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(ExplainDialogTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(ExplainDialogTimerHandle);
	}
}

FString UDasanWidget::GetStateMessage(EDasanState MainState, ETourState TourState, EAnswerState AnswerState,
	const FString& BuildingName, const FString& PlayerName, int32 CurrentPlayers, int32 MaxPlayers) const
{
	switch (MainState)
	{
	case EDasanState::Tour:
		{
			switch (TourState)
			{
			case ETourState::TourMove:
				// "%s 를 향해 이동 중입니다"
				if (!BuildingName.IsEmpty())
					return FString::Printf(TEXT("%s 으로 이동하시죠"), *BuildingName);
				return TEXT("이동중");

			case ETourState::TourWait:
				return TEXT("집결대기중");

			case ETourState::TourExplain:
				// "%s 에 대한 설명을 진행합니다"
				if (!BuildingName.IsEmpty())
					return FString::Printf(TEXT("%s"), *BuildingName);
				return TEXT("설명중");

			case ETourState::TourEnd:
				return TEXT("");  // 숨김
			default:
				return TEXT("");
			}
		}

	case EDasanState::Answer:
		{
			switch (AnswerState)
			{
			case EAnswerState::AnswerListen:
				// "%s 군주님의 질문을 듣고 있습니다"
				if (!PlayerName.IsEmpty())
					return FString::Printf(TEXT("%s 군주님의 질문을 듣고 있습니다"), *PlayerName);
				return TEXT("경청중");

			case EAnswerState::AnswerReply:
				return TEXT("답변중");
			case EAnswerState::AnswerEnd:
				return TEXT("");  // 숨김
			default:
				return TEXT("");
			}
		}

	default:
		return TEXT("");
	}
}



FString UDasanWidget::GetResourcePath(EDasanState MainState, ETourState TourState, EAnswerState AnswerState) const
{
	switch (MainState)
	{
	case EDasanState::Tour:
		{
			switch (TourState)
			{
			case ETourState::TourMove:
				return TEXT("/Game/CustomContents/UI/Texture/Dasan/dasan_move");
			case ETourState::TourWait:
				return TEXT("/Game/CustomContents/UI/Texture/Dasan/dasan_wait");
			case ETourState::TourExplain:
				return TEXT("/Game/CustomContents/UI/Texture/Dasan/dasan_ask");
			case ETourState::TourEnd:
				return TEXT("/Game/CustomContents/UI/Texture/Dasan/dasan_wait");
			default:
				return TEXT("/Game/CustomContents/UI/Texture/Dasan/dasan_wait");
			}
		}

	case EDasanState::Answer:
		{
			switch (AnswerState)
			{
			case EAnswerState::AnswerListen:
				return TEXT("/Game/CustomContents/UI/Texture/Dasan/dasan_listen");
			case EAnswerState::AnswerReply:
				return TEXT("/Game/CustomContents/UI/Texture/Dasan/dasan_ask");
			case EAnswerState::AnswerEnd:
				return TEXT("/Game/CustomContents/UI/Texture/Dasan/dasan_wait");
			default:
				return TEXT("/Game/CustomContents/UI/Texture/Dasan/dasan_wait");
			}
		}

	default:
		return TEXT("/Game/CustomContents/UI/Texture/Dasan/dasan_wait");
	}
}
