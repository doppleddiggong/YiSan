// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#include "ULoadingTransitionWidget.h"

#include "Components/TextBlock.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"

void ULoadingTransitionWidget::RefreshLoadingTip()
{
	const int32 Index = FMath::RandRange(0, LoadingTips.Num() - 1);
	LoadingTipText->SetText(FText::FromString(LoadingTips[Index]));
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
			RefreshLoadingTip();
		}
	}
}