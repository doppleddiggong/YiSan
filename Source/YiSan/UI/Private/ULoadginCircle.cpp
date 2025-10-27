// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "ULoadginCircle.h"

#include "GameLogging.h"
#include "UStateWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "HAL/CriticalSection.h"
#include "Misc/DateTime.h"
#include "Misc/ScopeLock.h"
#include "TimerManager.h"

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

	OnLoadingCountChanged.Broadcast(LoadingCount);
	UpdateVisibility();
}

void ULoadginCircle::HideLoading()
{
	const int32 OldCount = LoadingCount;
	LoadingCount = FMath::Max(0, LoadingCount - 1);

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

	if (bShouldBeVisible)
	{
		// 로딩 중일 때: 보이고, 터치 차단
		SetVisibility(ESlateVisibility::Visible);
		RootOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		// 로딩 완료 시: 안 보이지만 Hit Test 완전히 무시 (터치 이벤트 통과)
		SetVisibility(ESlateVisibility::HitTestInvisible);
		RootOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ULoadginCircle::AddToGameViewport(int32 ZOrder)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameViewportClient* ViewportClient = World->GetGameViewport())
		{
			// 이미 추가되어 있다면 제거 후 재추가
			if (IsInViewport())
				RemoveFromParent();

			// Game Viewport에 직접 추가 (레벨 전환 시에도 유지됨)
			ViewportClient->AddViewportWidgetContent(TakeWidget(), ZOrder);
		}
	}
}