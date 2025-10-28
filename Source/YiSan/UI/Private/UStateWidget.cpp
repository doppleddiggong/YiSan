// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UStateWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "HAL/CriticalSection.h"
#include "Misc/DateTime.h"
#include "Misc/ScopeLock.h"
#include "TimerManager.h"
#include "UBroadcastManager.h"
#include "UBuildingDetailData.h"
#include "UGameDataManager.h"

void UStateWidget::NativeConstruct()
{
    Super::NativeConstruct();

    QuestDisplayType = EBuildingType::None;

    LoadingSpinner->SetVisibility(ESlateVisibility::Hidden);
    SpectrumProgressBar->SetVisibility(ESlateVisibility::Hidden);
    
    QuestTargetText->SetText(FText::GetEmpty());
    QuestTargetText->SetVisibility(ESlateVisibility::Hidden);
    QuestTargetImage->SetVisibility(ESlateVisibility::Hidden);

    NearTargetText->SetText(FText::GetEmpty());
    NearTargetText->SetVisibility(ESlateVisibility::Hidden);
    NearTargetImage->SetVisibility(ESlateVisibility::Hidden);

    FocusTargetText->SetText(FText::GetEmpty());
    FocusTargetText->SetVisibility(ESlateVisibility::Hidden);
    FocusTargetImage->SetVisibility(ESlateVisibility::Hidden);
    
    if (UWorld* World = GetWorld())
        World->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UStateWidget::RefreshTimeText, TimeUpdateInterval, true);

    if ( auto EventManager = UBroadcastManager::Get(GetWorld()))
    {
        EventManager->OnNetworkWaitCount.AddDynamic(this, &UStateWidget::OnNetworkWaitCount);
        EventManager->OnAudioCapture.AddDynamic(this, &UStateWidget::OnAudioCapture);
        EventManager->OnAudioSpectrum.AddDynamic(this, &UStateWidget::OnAudioSpectrum);
        EventManager->OnUpdateQuest.AddDynamic(this, &UStateWidget::OnUpdateQuest);
        EventManager->OnNearBuilding.AddDynamic(this, &UStateWidget::OnNearBuilding);
        EventManager->OnFocusBuilding.AddDynamic(this, &UStateWidget::OnFocusBuilding);
    }
}

void UStateWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);

    Super::NativeDestruct();
}

void UStateWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    UpdateSpectrumVisual(InDeltaTime);
    UpdateLoadingSpinner(InDeltaTime);
}

void UStateWidget::RefreshTimeText()                                                                                                                                                                     
{
    const FDateTime Now = FDateTime::Now();
    const FString TimeString = Now.ToString(TEXT("%H:%M:%S"));
    const FText TimeText = FText::FromString(TimeString);                                                                                                                           

    CurrentTimeText->SetText(TimeText);                                                                                                                                       
}          

void UStateWidget::UpdateSpectrumVisual(float DeltaTime)
{
    if ( !SpectrumProgressBar->IsVisible())
        return;
    
    const float Delta = FMath::Max(DeltaTime, 0.0f);
    const float TargetValue = SpectrumDisplayValue;
    const float CurrentValue = SpectrumProgressBar->GetPercent();
    const float InterpSpeed = TargetValue > CurrentValue ? SpectrumRiseSpeed : SpectrumDecaySpeed;
    const float NewPercent = FMath::FInterpConstantTo(CurrentValue, TargetValue, Delta, InterpSpeed);
    SpectrumProgressBar->SetPercent(FMath::Clamp(NewPercent, 0.0f, 1.0f));
}

void UStateWidget::UpdateLoadingSpinner(float DeltaTime)
{
    if ( !LoadingSpinner->IsVisible())
        return;
    
    const float NewAngle = LoadingSpinner->GetRenderTransformAngle() + (SpinnerRotationSpeed * DeltaTime);
    LoadingSpinner->SetRenderTransformAngle(NewAngle);
}

void UStateWidget::OnNetworkWaitCount(int NetworkWaitCount)
{
    LoadingSpinner->SetVisibility( NetworkWaitCount > 0 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UStateWidget::OnAudioCapture(bool bRecording)
{
    SpectrumProgressBar->SetVisibility( bRecording ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UStateWidget::OnAudioSpectrum(float Spectrum)
{
    SpectrumDisplayValue = Spectrum;
}

void UStateWidget::OnUpdateQuest(EBuildingType InBuildingType)
{
    QuestDisplayType = InBuildingType;

    if (InBuildingType == EBuildingType::None)
    {
        QuestTargetText->SetText(FText::GetEmpty());
        QuestTargetText->SetVisibility(ESlateVisibility::Hidden);
        QuestTargetImage->SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    auto GameDataManager = UGameDataManager::Get(GetWorld());
    auto BuildingName  = GameDataManager->GetBuildingDataName(InBuildingType);
    QuestTargetText->SetText(FText::FromString(BuildingName));
    QuestTargetText->SetVisibility(ESlateVisibility::Visible);

    FBuildingAssetData AssetData;
    if ( GameDataManager->GetBuildingAssetData(InBuildingType, AssetData) )
    {
        UBuildingDetailData* DetailAsset = AssetData.BuildingDetailDataAsset.LoadSynchronous();
        if ( DetailAsset )
        {
            TSoftObjectPtr<UTexture2D> LoadedTexture;
            if ( DetailAsset->LoadThumbnailImage(LoadedTexture) && LoadedTexture.Get() )
            {
                QuestTargetImage->SetBrushFromTexture(LoadedTexture.Get());
                QuestTargetImage->SetVisibility(ESlateVisibility::Visible);
            }
        }
    }
}

void UStateWidget::OnNearBuilding(EBuildingType InBuildingType)
{
    if (InBuildingType == EBuildingType::None)
    {
        NearTargetText->SetText(FText::GetEmpty());
        NearTargetText->SetVisibility(ESlateVisibility::Hidden);
        NearTargetImage->SetVisibility(ESlateVisibility::Hidden);
        return;
    }
    
    auto GameDataManager = UGameDataManager::Get(GetWorld());
    auto BuildingName  = GameDataManager->GetBuildingDataName(InBuildingType);
    NearTargetText->SetText(FText::FromString(BuildingName));
    NearTargetText->SetVisibility(ESlateVisibility::Visible);

    FBuildingAssetData AssetData;
    if ( GameDataManager->GetBuildingAssetData(InBuildingType, AssetData) )
    {
        UBuildingDetailData* DetailAsset = AssetData.BuildingDetailDataAsset.LoadSynchronous();
        if ( DetailAsset )
        {
            TSoftObjectPtr<UTexture2D> LoadedTexture;
            if ( DetailAsset->LoadThumbnailImage(LoadedTexture) && LoadedTexture.Get() )
            {
                NearTargetImage->SetBrushFromTexture(LoadedTexture.Get());
                NearTargetImage->SetVisibility(ESlateVisibility::Visible);
            }
        }
    }
}

void UStateWidget::OnFocusBuilding(EBuildingType InBuildingType)
{
    if (InBuildingType == EBuildingType::None)
    {
        FocusTargetText->SetText(FText::GetEmpty());
        FocusTargetText->SetVisibility(ESlateVisibility::Hidden);
        FocusTargetImage->SetVisibility(ESlateVisibility::Hidden);
        return;
    }
    
    auto GameDataManager = UGameDataManager::Get(GetWorld());
    auto BuildingName  = GameDataManager->GetBuildingDataName(InBuildingType);
    FocusTargetText->SetText(FText::FromString(BuildingName));
    FocusTargetText->SetVisibility(ESlateVisibility::Visible);

    FBuildingAssetData AssetData;
    if ( GameDataManager->GetBuildingAssetData(InBuildingType, AssetData) )
    {
        UBuildingDetailData* DetailAsset = AssetData.BuildingDetailDataAsset.LoadSynchronous();
        if ( DetailAsset )
        {
            TSoftObjectPtr<UTexture2D> LoadedTexture;
            if ( DetailAsset->LoadThumbnailImage(LoadedTexture) && LoadedTexture.Get() )
            {
                FocusTargetImage->SetBrushFromTexture(LoadedTexture.Get());
                FocusTargetImage->SetVisibility(ESlateVisibility::Visible);
            }
        }
    }
}