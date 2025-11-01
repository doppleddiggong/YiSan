// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#include "ULoadingTransitionManager.h"

#include "FComponentHelper.h"
#include "GameLogging.h"
#include "UDialogManager.h"
#include "ULoadingTransitionWidget.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "YiSan/YiSan.h"

#define LOADINGTRANSITIONWIDGET_PATH TEXT("/Game/CustomContents/UI/WBP_LoadingTransition.WBP_LoadingTransition_C")

ULoadingTransitionManager::ULoadingTransitionManager()
{
	TransitionWidgetClass = FComponentHelper::LoadClass<ULoadingTransitionWidget>(LOADINGTRANSITIONWIDGET_PATH);
}

void ULoadingTransitionManager::EnsureWidgetForWorld(UWorld* World)
{
	if (World == nullptr || !World->IsGameWorld())
		return;

	const bool bIsValidWidget = IsValid(TransitionWidget);
	const bool bSameWorld = bIsValidWidget && TransitionWidget->GetWorld() == World;
	const bool bInViewport = bIsValidWidget && TransitionWidget->IsInViewport();

	if (bIsValidWidget && bSameWorld && bInViewport)
		return;

	if (bIsValidWidget)
	{
		if (TransitionWidget->IsInViewport())
			TransitionWidget->RemoveFromParent();

		TransitionWidget = nullptr;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			LocalPlayer = GameInstance->GetFirstGamePlayer();
		}

		if (LocalPlayer == nullptr)
		{
			PRINTLOG(TEXT("[LoadingTransitionManager] LocalPlayer is null"));
			return;
		}
	}

	APlayerController* PC = LocalPlayer->GetPlayerController(World);
	if (PC == nullptr)
		return;

	if (!TransitionWidgetClass)
	{
		PRINTLOG(TEXT("[LoadingTransitionManager] TransitionWidgetClass is null"));
		return;
	}

	if (auto NewWidget = CreateWidget<ULoadingTransitionWidget>(PC, TransitionWidgetClass))
	{
		NewWidget->AddToGameViewport(GameLayer::Loading);
		TransitionWidget = NewWidget;
	}
}

void ULoadingTransitionManager::ShowLoadingScreen()
{
	if (UWorld* World = GetWorld())
	{
		EnsureWidgetForWorld(World);

		if (auto DialogManager = UDialogManager::Get(this))
		{
			DialogManager->HideToastImmediately();
		}

		if (TransitionWidget)
		{
			if (!TransitionWidget->IsInViewport())
			{
				TransitionWidget->AddToGameViewport(GameLayer::Loading);
			}

			TransitionWidget->SetVisibility(ESlateVisibility::Visible);

			LatestReportedProgress = 0.0f;
			TransitionWidget->SetVisibility(ESlateVisibility::Visible);
			TransitionWidget->RefreshLoadingTip();

			if (HideTimerHandle.IsValid())
			{
				World->GetTimerManager().ClearTimer(HideTimerHandle);
			}

			LastShowTimestamp = FPlatformTime::Seconds();
			bHideRequested = false;
		}
	}
}

void ULoadingTransitionManager::HideLoadingScreen()
{
	if (!TransitionWidget)
		return;
	
	LatestReportedProgress = FMath::Clamp(LatestReportedProgress, 0.0f, 1.0f);


	if (UWorld* World = GetWorld())
	{
		if (HideTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(HideTimerHandle);
		}

		const double Elapsed = FPlatformTime::Seconds() - LastShowTimestamp;
		double Delay = 0.0;

		if (Elapsed < MinVisibleDurationSeconds)
		{
			Delay = MinVisibleDurationSeconds - Elapsed;
		}

		Delay = FMath::Max(Delay, HoldAfterCompletionSeconds);

		if (Delay > KINDA_SMALL_NUMBER)
		{
			bHideRequested = true;
			World->GetTimerManager().SetTimer(
					HideTimerHandle,
					this,
					&ULoadingTransitionManager::FinalizeHide,
					Delay,
					false);
			return;
		}
	}

	FinalizeHide();
	
}

void ULoadingTransitionManager::FinalizeHide()
{
	if (UWorld* World = GetWorld())
	{
		if (HideTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(HideTimerHandle);
		}
	}

	if (TransitionWidget)
	{
		if (TransitionWidget->IsInViewport())
		{
			TransitionWidget->RemoveFromParent();
		}

		TransitionWidget = nullptr;
	}

	bHideRequested = false;
}
