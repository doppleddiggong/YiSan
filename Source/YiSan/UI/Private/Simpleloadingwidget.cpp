// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#include "SimpleLoadingWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void USimpleLoadingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(0.0f);
	}

	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("로딩 중...")));
	}

	if (ProgressText)
	{
		ProgressText->SetText(GetPercentageText(0.0f));
	}
}

void USimpleLoadingWidget::UpdateProgress(float Progress, const FText& StatusTextValue)
{
	Progress = FMath::Clamp(Progress, 0.0f, 1.0f);

	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(Progress);
		
		// 진행률에 따라 색상 변경
		FLinearColor Color;
		if (Progress < 0.5f)
		{
			Color = FLinearColor::LerpUsingHSV(FLinearColor::Red, FLinearColor::Yellow, Progress * 2.0f);
		}
		else
		{
			Color = FLinearColor::LerpUsingHSV(FLinearColor::Yellow, FLinearColor::Green, (Progress - 0.5f) * 2.0f);
		}
		LoadingProgressBar->SetFillColorAndOpacity(Color);
	}

	if (ProgressText)
	{
		ProgressText->SetText(GetPercentageText(Progress));
	}

	if (StatusText)
	{
		StatusText->SetText(StatusTextValue);
	}
}

FText USimpleLoadingWidget::GetPercentageText(float Progress) const
{
	int32 Percentage = FMath::RoundToInt(Progress * 100.0f);
	return FText::FromString(FString::Printf(TEXT("%d%%"), Percentage));
}