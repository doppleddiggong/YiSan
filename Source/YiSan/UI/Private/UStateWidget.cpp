// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UStateWidget.h"

#include "FStateAudioAnalyzer.h"
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
    
    if (!IsDesignTime())
        StartAudioCapture();

    if (UWorld* World = GetWorld())
        World->GetTimerManager().SetTimer(TimeUpdateTimerHandle, this, &UStateWidget::RefreshTimeText, TimeUpdateInterval, true);

    if ( auto EventManager = UBroadcastManger::Get(GetWorld()))
    {
        EventManager->OnNetworkStateChanged.AddDynamic(this, &UStateWidget::OnNetworkStateChanged);
        EventManager->OnAudioCapture.AddDynamic(this, &UStateWidget::OnAudioCapture);
    }
}

void UStateWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
        World->GetTimerManager().ClearTimer(TimeUpdateTimerHandle);

    StopAudioCapture();

    Super::NativeDestruct();
}

void UStateWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    UpdateSpectrumVisual(InDeltaTime);

    if ( LoadingSpinner->IsVisible())
    {
        const float NewAngle = LoadingSpinner->GetRenderTransformAngle() + (SpinnerRotationSpeed * InDeltaTime);
        LoadingSpinner->SetRenderTransformAngle(NewAngle);
    }    
}

void UStateWidget::StartAudioCapture()
{
    if (!AudioAnalyzer.IsValid())
        AudioAnalyzer = MakeShared<FStateAudioAnalyzer>();

    if (AudioAnalyzer->Start(32))
    {
        SpectrumDisplayValue = 0.0f;
        SpectrumProgressBar->SetPercent(0.0f);
    }
}

void UStateWidget::StopAudioCapture()
{
    if (AudioAnalyzer.IsValid())
    {
        AudioAnalyzer->Stop();
        AudioAnalyzer.Reset();
    }

    SpectrumProgressBar->SetPercent(0.0f);
    SpectrumDisplayValue = 0.0f;
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
    UpdateSpectrumAnalyzer();

    const float Delta = FMath::Max(DeltaTime, 0.0f);
    const float TargetValue = SpectrumDisplayValue;
    const float CurrentValue = SpectrumProgressBar->GetPercent();
    const float InterpSpeed = TargetValue > CurrentValue ? SpectrumRiseSpeed : SpectrumDecaySpeed;
    const float NewPercent = FMath::FInterpConstantTo(CurrentValue, TargetValue, Delta, InterpSpeed);
    SpectrumProgressBar->SetPercent(FMath::Clamp(NewPercent, 0.0f, 1.0f));
}

void UStateWidget::UpdateSpectrumAnalyzer()
{
    if (!AudioAnalyzer.IsValid())
        return;

    TArray<float> LatestValues;
    if (!AudioAnalyzer->FetchSpectrum(LatestValues))
        return;

    if (LatestValues.Num() > 0)
    {
        float Average = 0.0f;
        for (const float Value : LatestValues)
        {
            Average += Value;
        }
        Average /= LatestValues.Num();
        SpectrumDisplayValue = FMath::Lerp(SpectrumDisplayValue, Average, SpectrumSmoothing);
    }
}

void UStateWidget::OnNetworkStateChanged(ENetworkState InState)
{
    if (NetworkState == InState)
        return;

    this->NetworkState = InState;
    LoadingSpinner->SetVisibility( NetworkState == ENetworkState::Requesting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UStateWidget::OnAudioCapture(bool bRecording)
{
    SpectrumProgressBar->SetVisibility( bRecording ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
