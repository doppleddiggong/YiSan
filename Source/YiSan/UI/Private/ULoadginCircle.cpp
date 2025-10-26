// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "ULoadginCircle.h"

#include "GameLogging.h"
#include "UStateWidget.h"

#include "UQuestManager.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "HAL/CriticalSection.h"
#include "Misc/DateTime.h"
#include "Misc/ScopeLock.h"
#include "TimerManager.h"
#include "UBroadcastManager.h"
#include "UGameDataManager.h"

void ULoadginCircle::NativeConstruct()
{
	Super::NativeConstruct();

	LoadingCount = 0;
	UpdateVisibility();
}

void ULoadginCircle::NativeDestruct()
{
	Super::NativeDestruct();

	// 델리게이트 정리
	OnLoadingCountChanged.Clear();
}

void ULoadginCircle::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateLoadingSpinner(InDeltaTime);
}

void ULoadginCircle::ShowLoading()
{
	LoadingCount++;
	PRINTLOG( TEXT("ULoadginCircle::ShowLoading() : LoadingCount = %d"), LoadingCount);
	OnLoadingCountChanged.Broadcast(LoadingCount);
	UpdateVisibility();
}

void ULoadginCircle::HideLoading()
{
	const int32 OldCount = LoadingCount;
	LoadingCount = FMath::Max(0, LoadingCount - 1);
	PRINTLOG( TEXT("ULoadginCircle::HideLoading() : LoadingCount %d -> %d"), OldCount, LoadingCount);
	OnLoadingCountChanged.Broadcast(LoadingCount);
	UpdateVisibility();
}

void ULoadginCircle::UpdateLoadingSpinner(float DeltaTime)
{
	if (!LoadingSpinner || !LoadingSpinner->IsVisible())
		return;

	const float NewAngle = LoadingSpinner->GetRenderTransformAngle() + (SpinnerRotationSpeed * DeltaTime);
	LoadingSpinner->SetRenderTransformAngle(NewAngle);
}

void ULoadginCircle::UpdateVisibility()
{
	if (!RootOverlay)
	{
		PRINTLOG( TEXT("ULoadginCircle::UpdateVisibility() : RootOverlay is nullptr!"));
		return;
	}

	const bool bShouldBeVisible = LoadingCount > 0;

	// RootOverlay만 제어하면 자식인 LoadingSpinner도 함께 제어됨
	const ESlateVisibility NewVisibility = bShouldBeVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	RootOverlay->SetVisibility(NewVisibility);

	PRINTLOG( TEXT("ULoadginCircle::UpdateVisibility() : LoadingCount = %d, Visibility = %s"),
		LoadingCount,
		bShouldBeVisible ? TEXT("Visible") : TEXT("Collapsed"));
}

void ULoadginCircle::AddToGameViewport(int32 ZOrder)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameViewportClient* ViewportClient = World->GetGameViewport())
		{
			// 이미 추가되어 있다면 제거 후 재추가
			if (IsInViewport())
			{
				RemoveFromParent();
			}

			// Game Viewport에 직접 추가 (레벨 전환 시에도 유지됨)
			ViewportClient->AddViewportWidgetContent(TakeWidget(), ZOrder);
		}
	}
}