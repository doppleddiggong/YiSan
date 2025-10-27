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

#define DIALOGWIDGET_PATH TEXT("/Game/CustomContents/UI/WBP_Dialog.WBP_Dialog_C")

UDialogManager::UDialogManager()
{
	DialogWidgetClass = FComponentHelper::LoadClass<UDialogWidget>(DIALOGWIDGET_PATH);
}

void UDialogManager::EnsureWidgetForWorld(UWorld* World)
{
	if (World == nullptr || !World->IsGameWorld())
		return;

	if (DialogWidget && DialogWidget->GetWorld() == World)
		return;

	if (DialogWidget)
	{
		DialogWidget->RemoveFromParent();
		DialogWidget = nullptr;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer == nullptr)
		return;

	// 멀티플레이 대응: PlayerController 사용                                                                                 
	APlayerController* PC = LocalPlayer->GetPlayerController(World);
	if (PC == nullptr)
		return;
	
	if (UDialogWidget* NewWidget = CreateWidget<UDialogWidget>(PC, DialogWidgetClass))
	{
		NewWidget->AddToViewport();
		DialogWidget = NewWidget;
	}
}

void UDialogManager::OnToastManager(const FString& Message)
{
	if (Message.IsEmpty())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		EnsureWidgetForWorld(World);

		if (DialogWidget)
		{
			DialogWidget->ShowDialog(Message);
		}
	}
}

void UDialogManager::Toast(UObject* WorldContextObject, const FString& Message)
{
	if (!WorldContextObject || Message.IsEmpty())
	{
		return;
	}

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (ULocalPlayer* LP = GI->GetFirstGamePlayer()) // 단일 플레이어 기준
			{
				if (UDialogManager* Dialog = LP->GetSubsystem<UDialogManager>())
				{
					Dialog->OnToastManager(Message);
				}
			}
		}
	}
}