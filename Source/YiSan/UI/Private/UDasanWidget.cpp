// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDasanWidget.h"
#include "ADasanActor.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PlayerController.h"

void UDasanWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDasanWidget::InitWidget(ADasanActor* InDasanActor)
{
	DasanActor = InDasanActor;
}

void UDasanWidget::UpdateDasanState(EDasanState MainState, ETourState TourState, EExplainState ExplainState, EAnswerState AnswerState)
{
	if (!Text_Msg)
		return;

	FString StateMessage = GetStateMessage(MainState, TourState, ExplainState, AnswerState);

	// 메시지가 비어있으면 위젯을 숨기는 대신, 텍스트만 비워 깜빡임을 방지합니다.
	Text_Msg->SetText(FText::FromString(StateMessage));
}

FString UDasanWidget::GetStateMessage(EDasanState MainState, ETourState TourState, EExplainState ExplainState, EAnswerState AnswerState) const
{
	switch (MainState)
	{
	case EDasanState::Tour:
		{
			switch (TourState)
			{
			case ETourState::TourMove:
				return TEXT("TourMove");
			case ETourState::TourWait:
				return TEXT("TourWait");
			case ETourState::TourEnd:
				return TEXT("");  // 숨김
			default:
				return TEXT("");
			}
		}

	case EDasanState::Explain:
		{
			switch (ExplainState)
			{
			case EExplainState::ExplainWait:
				return TEXT("");  // 숨김
			case EExplainState::ExplainIng:
				return TEXT("ExplainIng");
			default:
				return TEXT("");
			}
		}

	case EDasanState::Answer:
		{
			switch (AnswerState)
			{
			case EAnswerState::AnswerListen:
				return TEXT("AnswerListen");
			case EAnswerState::AnswerReply:
				return TEXT("AnswerReply");
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


