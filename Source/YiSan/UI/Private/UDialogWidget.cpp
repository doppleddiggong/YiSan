// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDialogWidget.h"
#include "UDelayTaskManager.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

void UDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (DialogBorder)
	{
		DialogBorder->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UDialogWidget::ShowDialog(FString InString)
{
	if (!DialogBorder || !DialogText)
		return;

	// 이전 타이머가 있으면 취소
	if (UWorld* World = GetWorld())
	{
		if (HideTimerHandle.IsValid())
			World->GetTimerManager().ClearTimer(HideTimerHandle);
	}

	DialogBorder->SetVisibility(ESlateVisibility::Visible);
	DialogBorder->SetBrushColor(ActivateColor);
	DialogText->SetText(FText::FromString(InString));

	// 5초 후 숨기는 타이머 설정
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			HideTimerHandle,
			[this]()
			{
				DialogText->SetText(FText::GetEmpty());
				DialogBorder->SetBrushColor(DeactivateColor);
			},
			5.0f,
			false
		);
	}
}
