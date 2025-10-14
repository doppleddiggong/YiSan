// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UStateWidget.h"

#include "GameLogging.h"
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
#include "UBroadcastManger.h"

void UStateWidget::NativeConstruct()
{
    Super::NativeConstruct();

    LoadingSpinner->SetVisibility(ESlateVisibility::Hidden);
    SpectrumProgressBar->SetVisibility(ESlateVisibility::Hidden);
    
    if (UWorld* World = GetWorld())
        World->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UStateWidget::RefreshTimeText, TimeUpdateInterval, true);

    if ( auto EventManager = UBroadcastManger::Get(GetWorld()))
    {
        EventManager->OnNetworkWaitCount.AddDynamic(this, &UStateWidget::OnNetworkWaitCount);
        EventManager->OnAudioCapture.AddDynamic(this, &UStateWidget::OnAudioCapture);
        EventManager->OnAudioSpectrum.AddDynamic(this, &UStateWidget::OnAudioSpectrum);
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

void UStateWidget::OnFocusBuilding(EBuildingType InBuildingType)
{
    if ( BuildingType == InBuildingType )
        return;

    BuildingType = InBuildingType;
    
    FText DisplayName = StaticEnum<EBuildingType>()->GetDisplayNameTextByValue(static_cast<int64>(BuildingType));
    PRINT_STRING(TEXT("%s"), *DisplayName.ToString());
}
	