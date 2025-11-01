// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDialogManager.h"

#include "FComponentHelper.h"
#include "UDialogWidget.h"

#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "YiSan/YiSan.h"

#define DIALOGWIDGET_PATH TEXT("/Game/CustomContents/UI/WBP_Dialog.WBP_Dialog_C")

UDialogManager::UDialogManager()
{
	DialogWidgetClass = FComponentHelper::LoadClass<UDialogWidget>(DIALOGWIDGET_PATH);
}

void UDialogManager::EnsureWidgetForWorld(UWorld* World)
{
	if (World == nullptr || !World->IsGameWorld())
		return;

	// 위젯이 유효하고 같은 월드이며, 뷰포트에 추가되어 있는지 확인
	if (IsValid(DialogWidget) &&
		DialogWidget->GetWorld() == World &&
		DialogWidget->IsInViewport())
	{
		return;
	}

	// 기존 위젯이 있으면 정리
	if (DialogWidget)
	{
		if (DialogWidget->IsInViewport())
			DialogWidget->RemoveFromParent();
		
		DialogWidget = nullptr;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer == nullptr)
		return;

	APlayerController* PC = LocalPlayer->GetPlayerController(World);
	if (PC == nullptr)
		return;

	if (!DialogWidgetClass)
		return;

	UDialogWidget* NewWidget = CreateWidget<UDialogWidget>(PC, DialogWidgetClass);
	if (!NewWidget)
		return;

	NewWidget->AddToViewport(GameLayer::ToastManager);
	DialogWidget = NewWidget;
}

void UDialogManager::ShowToast(const FString& Message)
{
	if (Message.IsEmpty())
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	EnsureWidgetForWorld(World);

	if (DialogWidget)
		DialogWidget->ShowDialog(Message);
}

void UDialogManager::HideToastImmediately()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	if (!DialogWidget)
		return;

	if (DialogWidget->GetWorld() != World)
		return;

	DialogWidget->HideDialogImmediately();
}