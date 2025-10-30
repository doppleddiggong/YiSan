// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#include "ULoadingTransitionWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"


void ULoadingTransitionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ProgressBar->SetPercent(0.0f);

	UpdateStatus( TEXT("준비중"));
	UpdateLoadingTip(0);
}

void ULoadingTransitionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateLoadingSpinner(InDeltaTime);
	UpdateLoadingTip(InDeltaTime);
}

void ULoadingTransitionWidget::UpdateLoadingSpinner(float DeltaTime) const
{
	if (!LoadingSpinner || !LoadingSpinner->IsVisible())
		return;

	const float NewAngle = LoadingSpinner->GetRenderTransformAngle() + (SpinnerRotationSpeed * DeltaTime);
	LoadingSpinner->SetRenderTransformAngle(NewAngle);
}

void ULoadingTransitionWidget::UpdateLoadingTip(float DeltaTime)
{
	TipElapsed += DeltaTime;
	if (TipElapsed < TipInterval)
		return;

	TipElapsed = 0.f;

	if (LoadingTipText && LoadingTips.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, LoadingTips.Num() - 1);
		LoadingTipText->SetText(FText::FromString(LoadingTips[Index]));
	}
}

void ULoadingTransitionWidget::UpdateStatus(const FString& Status)
{
	StatusText->SetText(FText::FromString(Status));
}

void ULoadingTransitionWidget::UpdateProgress(float Progress)
{
	ProgressBar->SetPercent(Progress);

	int32 Percentage = FMath::RoundToInt(Progress * 100.0f);
	ProgressText->SetText( FText::FromString(FString::Printf(TEXT("%d%%"), Percentage)) );

}



void ULoadingTransitionWidget::AddToGameViewport(int32 ZOrder)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameViewportClient* ViewportClient = World->GetGameViewport())
		{
			if (IsInViewport())
			{
				RemoveFromParent();
			}

			AddToViewport(ZOrder);
		}
	}
}