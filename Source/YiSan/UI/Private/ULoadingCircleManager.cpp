// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ULoadingCircleManager.h"

#include "FComponentHelper.h"
#include "ULoadginCircle.h"

#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

#define LOADINGCIRCLEWIDGET_PATH TEXT("/Game/CustomContents/UI/WBP_NetworkLoading.WBP_NetworkLoading_C")

ULoadingCircleManager::ULoadingCircleManager()
{
	LoadingCircleWidgetClass = FComponentHelper::LoadClass<ULoadginCircle>(LOADINGCIRCLEWIDGET_PATH);
}

void ULoadingCircleManager::EnsureWidgetForWorld(UWorld* World)
{
	if (World == nullptr || !World->IsGameWorld())
		return;

	// 위젯이 유효하고 같은 월드이며, 뷰포트에 추가되어 있는지 확인
	if (IsValid(LoadingCircleWidget) &&
		LoadingCircleWidget->GetWorld() == World &&
		LoadingCircleWidget->IsInViewport())
	{
		return;
	}

	// 기존 위젯이 있으면 정리
	if (LoadingCircleWidget)
	{
		if (LoadingCircleWidget->IsInViewport())
			LoadingCircleWidget->RemoveFromParent();

		LoadingCircleWidget = nullptr;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer == nullptr)
		return;

	// 멀티플레이 대응: PlayerController 사용
	APlayerController* PC = LocalPlayer->GetPlayerController(World);
	if (PC == nullptr)
		return;

	if (!LoadingCircleWidgetClass)
		return;

	if (ULoadginCircle* NewWidget = CreateWidget<ULoadginCircle>(PC, LoadingCircleWidgetClass))
	{
		// Game Viewport에 추가하여 레벨 전환 시에도 유지
		NewWidget->AddToGameViewport(1000);
		LoadingCircleWidget = NewWidget;
	}
}

void ULoadingCircleManager::IncrementLoading()
{
	if (UWorld* World = GetWorld())
	{
		EnsureWidgetForWorld(World);

		if (LoadingCircleWidget)
		{
			LoadingCircleWidget->ShowLoading();
		}
	}
}

void ULoadingCircleManager::DecrementLoading()
{
	if (UWorld* World = GetWorld())
	{
		EnsureWidgetForWorld(World);

		if (LoadingCircleWidget)
		{
			LoadingCircleWidget->HideLoading();
		}
	}
}

int32 ULoadingCircleManager::GetLoadingCount() const
{
	if (LoadingCircleWidget)
	{
		return LoadingCircleWidget->GetLoadingCount();
	}
	return 0;
}

void ULoadingCircleManager::Increase(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return;
	}

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (ULocalPlayer* LP = GI->GetFirstGamePlayer())
			{
				if (ULoadingCircleManager* Manager = LP->GetSubsystem<ULoadingCircleManager>())
				{
					Manager->IncrementLoading();
				}
			}
		}
	}
}

void ULoadingCircleManager::Decrease(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return;
	}

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (ULocalPlayer* LP = GI->GetFirstGamePlayer())
			{
				if (ULoadingCircleManager* Manager = LP->GetSubsystem<ULoadingCircleManager>())
				{
					Manager->DecrementLoading();
				}
			}
		}
	}
}
