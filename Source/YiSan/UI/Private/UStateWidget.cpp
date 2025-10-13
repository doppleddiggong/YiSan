// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UStateWidget.h"

#include "FStateAudioAnalyzer.h"

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

void UStateWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    RefreshNetworkWidgets();
    HandleTimeUpdate();
}

void UStateWidget::NativeConstruct()
{
    Super::NativeConstruct();

    RefreshNetworkWidgets();

    HandleTimeUpdate();

    if (!IsDesignTime())
    {
        EnsureTimeTimer();

        if (bAutoStartAudioCapture)
            StartAudioCapture();
    }

    if ( auto EventManager = UBroadcastManger::Get(GetWorld()))
        EventManager->OnNetworkStateChanged.AddDynamic(this, &UStateWidget::OnNetworkStateChanged);
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

    if (TimeUpdateInterval <= 0.0f)
        HandleTimeUpdate();

    UpdateSpectrumVisualization(InDeltaTime);
}

void UStateWidget::SetUseSystemTime(bool bInUseSystemTime)
{
    bUseSystemTime = bInUseSystemTime;
    HandleTimeUpdate();
}

void UStateWidget::StartAudioCapture()
{
    if (!AudioAnalyzer.IsValid())
        AudioAnalyzer = MakeShared<FStateAudioAnalyzer>();

    if (AudioAnalyzer->Start(32))
    {
        SpectrumDisplayValue = 0.0f;
        if (SpectrumProgressBar)
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

    if (SpectrumProgressBar)
        SpectrumProgressBar->SetPercent(0.0f);

    SpectrumDisplayValue = 0.0f;
}



void UStateWidget::EnsureTimeTimer()
{
    if (TimeUpdateInterval <= 0.0f)
        return;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(TimeUpdateTimerHandle, this, &UStateWidget::HandleTimeUpdate, TimeUpdateInterval, true);
    }
}

void UStateWidget::HandleTimeUpdate()
{
    RefreshTimeTexts();
}

void UStateWidget::RefreshTimeTexts()
{
    bool bIsDaytime = true;
    float SecondsUntilTransition = 0.0f;
    float DayNightProgress = EvaluateGameTimeProgress(SecondsUntilTransition, bIsDaytime);

    FText TimeText = FText::GetEmpty();
    BuildGameClockDisplay(TimeText);

    if (CurrentTimeText)
        CurrentTimeText->SetText(TimeText);

    UpdateDayNightWidgets(DayNightProgress, bIsDaytime, SecondsUntilTransition);
}

float UStateWidget::EvaluateGameTimeProgress(float& OutSecondsUntilTransition, bool& bOutIsDaytime) const
{
    OutSecondsUntilTransition = 0.0f;
    bOutIsDaytime = true;

    const float TotalLength = FMath::Max(GameDayLengthSeconds, 0.1f);
    const float DayDuration = TotalLength * FMath::Clamp(DayRatio, 0.1f, 0.9f);
    const float NightDuration = TotalLength - DayDuration;

    float WorldSeconds = 0.0f;
    if (const UWorld* World = GetWorld())
        WorldSeconds = World->GetTimeSeconds();

    const float CyclePosition = FMath::Fmod(WorldSeconds, TotalLength);
    if (CyclePosition < 0.0f)
    {
        OutSecondsUntilTransition = DayDuration;
        bOutIsDaytime = true;
        return 0.0f;
    }

    float Progress = CyclePosition / TotalLength;
    if (CyclePosition <= DayDuration)
    {
        bOutIsDaytime = true;
        OutSecondsUntilTransition = DayDuration - CyclePosition;
    }
    else
    {
        bOutIsDaytime = false;
        const float NightElapsed = CyclePosition - DayDuration;
        OutSecondsUntilTransition = FMath::Max(NightDuration - NightElapsed, 0.0f);
    }

    return FMath::Clamp(Progress, 0.0f, 1.0f);
}

void UStateWidget::BuildGameClockDisplay(FText& OutCurrentTimeText) const
{
    if (bUseSystemTime)
    {
        const FDateTime Now = FDateTime::Now();
        const FString TimeString = Now.ToString(TEXT("HH:mm:ss"));
        OutCurrentTimeText = FText::FromString(TimeString);
    }
    else
    {
        float WorldSeconds = 0.0f;
        if (const UWorld* World = GetWorld())
            WorldSeconds = World->GetTimeSeconds();

        const float DayLength = FMath::Max(GameDayLengthSeconds, 0.1f);
        const int32 DayIndex = static_cast<int32>(WorldSeconds / DayLength);
        const float CycleSeconds = FMath::Fmod(WorldSeconds, DayLength);
        const float GameSecondsInDay = (CycleSeconds / DayLength) * 24.0f * 3600.0f;

        const int32 Hours = static_cast<int32>(GameSecondsInDay) / 3600;
        const int32 Minutes = (static_cast<int32>(GameSecondsInDay) % 3600) / 60;
        const int32 Seconds = static_cast<int32>(GameSecondsInDay) % 60;

        const FString TimeString = FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds);
        OutCurrentTimeText = FText::Format(
            NSLOCTEXT("StateWidget", "GameClock", "Day {0}  {1}"),
            FText::AsNumber(DayIndex + 1),
            FText::FromString(TimeString)
        );
    }
}

void UStateWidget::UpdateDayNightWidgets(float DayNightProgress, bool bIsDaytime, float SecondsToTransition)
{
    if (DayNightProgressBar)
        DayNightProgressBar->SetPercent(DayNightProgress);

    if (DayNightStatusText)
        DayNightStatusText->SetText(BuildPhaseStatusText(SecondsToTransition, bIsDaytime));
}

void UStateWidget::RefreshNetworkWidgets()
{
    if (NetworkStatusText)
        NetworkStatusText->SetText(FText::FromString(GetNetworkStateString(NetworkState)));

    if (LoadingSpinner)
        LoadingSpinner->SetVisibility(ShouldSpinnerBeVisible(NetworkState) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UStateWidget::UpdateSpectrumVisualization(float DeltaTime)
{
    UpdateSpectrumFromAnalyzer();

    if (!SpectrumProgressBar)
        return;

    const float Delta = FMath::Max(DeltaTime, 0.0f);
    const float TargetValue = SpectrumDisplayValue;
    const float CurrentValue = SpectrumProgressBar->GetPercent();
    const float InterpSpeed = TargetValue > CurrentValue ? SpectrumRiseSpeed : SpectrumDecaySpeed;
    const float NewPercent = FMath::FInterpConstantTo(CurrentValue, TargetValue, Delta, InterpSpeed);
    SpectrumProgressBar->SetPercent(FMath::Clamp(NewPercent, 0.0f, 1.0f));
}

void UStateWidget::UpdateSpectrumFromAnalyzer()
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
    RefreshNetworkWidgets();
}

FText UStateWidget::BuildPhaseStatusText(float SecondsUntilTransition, bool bIsDaytime) const
{
    const int32 MinutesLeft = static_cast<int32>(SecondsUntilTransition) / 60;
    const int32 SecondsLeft = static_cast<int32>(SecondsUntilTransition) % 60;
    return FText::Format(
        NSLOCTEXT("StateWidget", "PhaseStatus", "{0} | 전환까지 {1:02d}:{2:02d}"),
        bIsDaytime ? NSLOCTEXT("StateWidget", "PhaseDay", "낮") : NSLOCTEXT("StateWidget", "PhaseNight", "밤"),
        MinutesLeft,
        SecondsLeft
    );
}