// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDialogWidgetService.h"

#include "GameLogging.h"
#include "UDialogWidget.h"
#include "UBroadcastManager.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

UDialogWidgetService::UDialogWidgetService()
{
        DialogWidgetClass = UDialogWidget::StaticClass();
}

void UDialogWidgetService::Initialize(FSubsystemCollectionBase& Collection)
{
        Super::Initialize(Collection);

        if (!DialogWidgetClass)
        {
                DialogWidgetClass = UDialogWidget::StaticClass();
        }

        BindBroadcastDelegates();

        if (!WorldBeginPlayHandle.IsValid())
        {
                WorldBeginPlayHandle = FWorldDelegates::OnWorldBeginPlay.AddUObject(this, &UDialogWidgetService::HandleWorldBeginPlay);
        }

        if (!WorldCleanupHandle.IsValid())
        {
                WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &UDialogWidgetService::HandleWorldCleanup);
        }

        if (UGameInstance* GameInstance = GetGameInstance())
        {
                if (UWorld* World = GameInstance->GetWorld())
                {
                        HandleWorldBeginPlay(World);
                }
        }
}

void UDialogWidgetService::Deinitialize()
{
        UnbindBroadcastDelegates();

        if (WorldBeginPlayHandle.IsValid())
        {
                FWorldDelegates::OnWorldBeginPlay.Remove(WorldBeginPlayHandle);
                WorldBeginPlayHandle.Reset();
        }

        if (WorldCleanupHandle.IsValid())
        {
                FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
                WorldCleanupHandle.Reset();
        }

        TeardownWidget();

        Super::Deinitialize();
}

void UDialogWidgetService::RequestDialog(const FString& Message)
{
        HandleToastMessage(Message);
}

TSubclassOf<UDialogWidget> UDialogWidgetService::GetDialogWidgetClass() const
{
        return DialogWidgetClass ? DialogWidgetClass : UDialogWidget::StaticClass();
}

void UDialogWidgetService::EnsureWidgetForWorld(UWorld* World)
{
        if (World == nullptr || !World->IsGameWorld())
        {
                return;
        }

        if (DialogWidget && DialogWidget->GetWorld() == World)
        {
                return;
        }

        if (DialogWidget)
        {
                DialogWidget->RemoveFromParent();
                DialogWidget = nullptr;
        }

        UGameInstance* GameInstance = GetGameInstance();
        if (GameInstance == nullptr)
        {
                return;
        }

        ULocalPlayer* LocalPlayer = GameInstance->GetFirstGamePlayer();
        if (LocalPlayer == nullptr)
        {
                UE_LOG(LogCoffee, Verbose, TEXT("DialogWidgetService: No local player available to create dialog widget."));
                return;
        }

        if (UDialogWidget* NewWidget = CreateWidget<UDialogWidget>(LocalPlayer, GetDialogWidgetClass()))
        {
                NewWidget->AddToViewport();
                DialogWidget = NewWidget;
        }
        else
        {
                UE_LOG(LogCoffee, Warning, TEXT("DialogWidgetService: Failed to create dialog widget instance."));
        }
}

void UDialogWidgetService::HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
        if (DialogWidget && DialogWidget->GetWorld() == World)
        {
                TeardownWidget();
        }
}

void UDialogWidgetService::HandleWorldBeginPlay(UWorld* World)
{
        if (World == nullptr || World->GetGameInstance() != GetGameInstance())
        {
                return;
        }

        EnsureWidgetForWorld(World);
}

void UDialogWidgetService::HandleToastMessage(const FString& InMessage)
{
        if (InMessage.IsEmpty())
        {
                return;
        }

        UGameInstance* GameInstance = GetGameInstance();
        if (GameInstance == nullptr)
        {
                return;
        }

        UWorld* World = GameInstance->GetWorld();
        EnsureWidgetForWorld(World);

        if (DialogWidget)
        {
                DialogWidget->ShowDialog(InMessage);
        }
}

void UDialogWidgetService::BindBroadcastDelegates()
{
        if (UBroadcastManager* BroadcastManager = UBroadcastManager::Get(this))
        {
                BroadcastManager->OnToastMessage.RemoveDynamic(this, &UDialogWidgetService::HandleToastMessage);
                BroadcastManager->OnToastMessage.AddDynamic(this, &UDialogWidgetService::HandleToastMessage);
        }
}

void UDialogWidgetService::UnbindBroadcastDelegates()
{
        if (UBroadcastManager* BroadcastManager = UBroadcastManager::Get(this))
        {
                BroadcastManager->OnToastMessage.RemoveDynamic(this, &UDialogWidgetService::HandleToastMessage);
        }
}

void UDialogWidgetService::TeardownWidget()
{
        if (DialogWidget)
        {
                DialogWidget->RemoveFromParent();
                DialogWidget = nullptr;
        }
}
