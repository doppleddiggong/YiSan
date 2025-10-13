// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UStateWidget.h"

#include "AudioCapture.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "HAL/CriticalSection.h"
#include "Misc/DateTime.h"
#include "Misc/ScopeLock.h"
#include "Layout/SlateChildSize.h"
#include "TimerManager.h"

namespace
{
        constexpr float Pi = 3.14159265358979323846f;

        static FString GetNetworkStateString(ENetworkInteractionState InState)
        {
                switch (InState)
                {
                        case ENetworkInteractionState::Idle:
                                return TEXT("대기중");
                        case ENetworkInteractionState::Waiting:
                                return TEXT("대기중");
                        case ENetworkInteractionState::Requesting:
                                return TEXT("요청중");
                        case ENetworkInteractionState::Responding:
                                return TEXT("응답중");
                        case ENetworkInteractionState::Completed:
                                return TEXT("완료");
                        case ENetworkInteractionState::Failed:
                                return TEXT("실패");
                        default:
                                return TEXT("알수없음");
                }
        }

        static bool ShouldSpinnerBeVisible(ENetworkInteractionState InState)
        {
                return InState == ENetworkInteractionState::Waiting
                        || InState == ENetworkInteractionState::Requesting
                        || InState == ENetworkInteractionState::Responding;
        }
}

class FStateAudioAnalyzer : public TSharedFromThis<FStateAudioAnalyzer>
{
public:
        FStateAudioAnalyzer() = default;

        bool Start(int32 InBarCount)
        {
                if (InBarCount <= 0)
                        return false;

                BarCount = InBarCount;
                LatestSpectrum.Init(0.0f, BarCount);
                PendingSpectrum.Init(0.0f, BarCount);
                FFTSize = 256;

                if (!AudioCapture)
                        AudioCapture = MakeUnique<Audio::FAudioCapture>();

                Audio::FAudioCaptureDeviceParams Params;
                Params.DeviceIndex = 0;
                Params.NumInputChannels = 1;

                const int32 DesiredFrameSize = FFTSize;

                bool bOpened = AudioCapture->OpenAudioCaptureStream(
                        Params,
                        [this](const void* InAudio, int32 InNumFrames, int32 InNumChannels, int32 InSampleRate, double, bool)
                        {
                                HandleAudio(static_cast<const float*>(InAudio), InNumFrames, InNumChannels, InSampleRate);
                        },
                        DesiredFrameSize
                );

                if (!bOpened)
                        return false;

                return AudioCapture->StartStream();
        }

        void Stop()
        {
                if (AudioCapture)
                {
                        AudioCapture->StopStream();
                        AudioCapture->CloseStream();
                }
        }

        bool FetchSpectrum(TArray<float>& OutSpectrum)
        {
                FScopeLock Lock(&SpectrumGuard);
                if (!bPending)
                        return false;

                OutSpectrum = PendingSpectrum;
                bPending = false;
                return true;
        }

private:
        void HandleAudio(const float* InAudio, int32 InNumFrames, int32 InNumChannels, int32 InSampleRate)
        {
                if (!InAudio || InNumFrames <= 0)
                        return;

                const int32 SampleCount = InNumFrames * InNumChannels;
                if (SampleCount <= 0)
                        return;

                if (FFTInput.Num() != FFTSize)
                        FFTInput.SetNumZeroed(FFTSize);

                const int32 CopyCount = FMath::Min(SampleCount, FFTSize);
                for (int32 Index = 0; Index < CopyCount; ++Index)
                {
                        const int32 SampleIndex = Index * InNumChannels;
                        float Accumulated = 0.0f;
                        for (int32 Channel = 0; Channel < InNumChannels; ++Channel)
                        {
                                Accumulated += InAudio[SampleIndex + Channel];
                        }

                        FFTInput[Index] = Accumulated / static_cast<float>(InNumChannels);
                }

                for (int32 Index = CopyCount; Index < FFTSize; ++Index)
                        FFTInput[Index] = 0.0f;

                ApplyHannWindow();
                ComputeSpectrum(InSampleRate);
        }

        void ApplyHannWindow()
        {
                if (Window.Num() != FFTSize)
                {
                        Window.SetNum(FFTSize);
                        for (int32 Index = 0; Index < FFTSize; ++Index)
                                Window[Index] = 0.5f * (1.0f - FMath::Cos(2.0f * Pi * static_cast<float>(Index) / (FFTSize - 1)));
                }

                for (int32 Index = 0; Index < FFTSize; ++Index)
                        FFTInput[Index] *= Window[Index];
        }

        void ComputeSpectrum(int32 SampleRate)
        {
                if (SampleRate <= 0)
                        return;

                if (Magnitude.Num() != FFTSize / 2)
                        Magnitude.SetNumZeroed(FFTSize / 2);

                for (int32 Bin = 0; Bin < FFTSize / 2; ++Bin)
                {
                        float Real = 0.0f;
                        float Imaginary = 0.0f;
                        const float AngleStep = -2.0f * Pi * static_cast<float>(Bin) / static_cast<float>(FFTSize);

                        for (int32 Time = 0; Time < FFTSize; ++Time)
                        {
                                const float Value = FFTInput[Time];
                                const float Angle = AngleStep * static_cast<float>(Time);
                                Real += Value * FMath::Cos(Angle);
                                Imaginary += Value * FMath::Sin(Angle);
                        }

                        const float Power = Real * Real + Imaginary * Imaginary;
                        Magnitude[Bin] = Power;
                }

                const float MinFrequency = 20.0f;
                const float MaxFrequency = 20000.0f;
                const float LogMin = FMath::Loge(MinFrequency);
                const float LogMax = FMath::Loge(MaxFrequency);

                TArray<float> Accum;
                TArray<int32> Counts;
                Accum.Init(0.0f, BarCount);
                Counts.Init(0, BarCount);

                for (int32 Bin = 1; Bin < FFTSize / 2; ++Bin)
                {
                        const float Frequency = (static_cast<float>(Bin) * static_cast<float>(SampleRate)) / static_cast<float>(FFTSize);
                        if (Frequency < MinFrequency || Frequency > MaxFrequency)
                                continue;

                        const float Normalized = (FMath::Loge(Frequency) - LogMin) / (LogMax - LogMin);
                        const int32 BarIndex = FMath::Clamp(static_cast<int32>(Normalized * static_cast<float>(BarCount)), 0, BarCount - 1);
                        Accum[BarIndex] += Magnitude[Bin];
                        Counts[BarIndex]++;
                }

                for (int32 BarIndex = 0; BarIndex < BarCount; ++BarIndex)
                {
                        float Value = Counts[BarIndex] > 0 ? Accum[BarIndex] / static_cast<float>(Counts[BarIndex]) : 0.0f;
                        Value = 10.0f * FMath::LogX(10.0f, FMath::Max(Value, 1e-6f));
                        Value = FMath::GetMappedRangeValueClamped(FVector2D(-60.0f, 0.0f), FVector2D(0.0f, 1.0f), Value);
                        LatestSpectrum[BarIndex] = Value;
                }

                {
                        FScopeLock Lock(&SpectrumGuard);
                        PendingSpectrum = LatestSpectrum;
                        bPending = true;
                }
        }

private:
        TUniquePtr<Audio::FAudioCapture> AudioCapture;

        int32 FFTSize = 256;
        int32 BarCount = 32;

        TArray<float> FFTInput;
        TArray<float> Window;
        TArray<float> Magnitude;
        TArray<float> LatestSpectrum;

        TArray<float> PendingSpectrum;
        FCriticalSection SpectrumGuard;
        bool bPending = false;
};

UStateWidget::UStateWidget(const FObjectInitializer& ObjectInitializer)
        : Super(ObjectInitializer)
{
        bCanEverTick = true;
}

void UStateWidget::NativePreConstruct()
{
        Super::NativePreConstruct();

        InitializeSpectrumBars();
        RefreshNetworkWidgets();
        HandleTimeUpdate();
}

void UStateWidget::NativeConstruct()
{
        Super::NativeConstruct();

        InitializeSpectrumBars();
        RefreshNetworkWidgets();

        HandleTimeUpdate();

        if (!IsDesignTime())
        {
                EnsureTimeTimer();

                if (bAutoStartAudioCapture)
                        StartAudioCapture();
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

        if (TimeUpdateInterval <= 0.0f)
                HandleTimeUpdate();

        UpdateSpectrumVisualization(InDeltaTime);
}

void UStateWidget::SetUseSystemTime(bool bInUseSystemTime)
{
        bUseSystemTime = bInUseSystemTime;
        HandleTimeUpdate();
}

void UStateWidget::SetNetworkInteractionState(ENetworkInteractionState InState)
{
        if (NetworkState == InState)
                return;

        NetworkState = InState;
        RefreshNetworkWidgets();
        OnNetworkInteractionStateChanged.Broadcast(NetworkState);
}

void UStateWidget::StartAudioCapture()
{
        if (!AudioAnalyzer.IsValid())
                AudioAnalyzer = MakeShared<FStateAudioAnalyzer>();

        if (AudioAnalyzer->Start(SpectrumBarCount))
        {
                SpectrumDisplayValues.Init(0.0f, SpectrumBarCount);
                for (TWeakObjectPtr<UProgressBar>& BarPtr : SpectrumBars)
                        if (BarPtr.IsValid())
                                BarPtr->SetPercent(0.0f);
        }
}

void UStateWidget::StopAudioCapture()
{
        if (AudioAnalyzer.IsValid())
        {
                AudioAnalyzer->Stop();
                AudioAnalyzer.Reset();
        }

        for (TWeakObjectPtr<UProgressBar>& BarPtr : SpectrumBars)
                if (BarPtr.IsValid())
                        BarPtr->SetPercent(0.0f);

        SpectrumDisplayValues.Init(0.0f, SpectrumBarCount);
}

void UStateWidget::RebuildSpectrumBars()
{
        const bool bWasDesignTime = IsDesignTime();

        if (!bWasDesignTime)
                StopAudioCapture();

        InitializeSpectrumBars();

        if (!bWasDesignTime && bAutoStartAudioCapture)
                StartAudioCapture();
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

void UStateWidget::InitializeSpectrumBars()
{
        SpectrumBars.Reset();
        SpectrumDisplayValues.Reset();

        if (!SpectrumContainer || !WidgetTree)
                return;

        SpectrumContainer->ClearChildren();
        SpectrumBarCount = FMath::Max(1, SpectrumBarCount);
        SpectrumBars.Reserve(SpectrumBarCount);
        SpectrumDisplayValues.Init(0.0f, SpectrumBarCount);

        for (int32 Index = 0; Index < SpectrumBarCount; ++Index)
        {
                UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
                if (!Bar)
                        continue;

                Bar->SetPercent(0.0f);
                Bar->SetFillColorAndOpacity(FLinearColor(0.1f, 0.7f, 1.0f, 1.0f));
                Bar->SetIsMarquee(false);

                UHorizontalBoxSlot* Slot = SpectrumContainer->AddChildToHorizontalBox(Bar);
                if (Slot)
                {
                        Slot->SetPadding(FMargin(1.0f, 0.0f));
                        Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                        Slot->SetHorizontalAlignment(HAlign_Fill);
                        Slot->SetVerticalAlignment(VAlign_Fill);
                }

                SpectrumBars.Add(Bar);
        }
}

void UStateWidget::UpdateSpectrumVisualization(float DeltaTime)
{
        UpdateSpectrumFromAnalyzer();

        const float Delta = FMath::Max(DeltaTime, 0.0f);
        for (int32 Index = 0; Index < SpectrumBars.Num(); ++Index)
        {
                if (!SpectrumBars[Index].IsValid())
                        continue;

                const float TargetValue = Index < SpectrumDisplayValues.Num() ? SpectrumDisplayValues[Index] : 0.0f;
                UProgressBar* Bar = SpectrumBars[Index].Get();
                if (!Bar)
                        continue;

                const float CurrentValue = Bar->GetPercent();
                const float InterpSpeed = TargetValue > CurrentValue ? SpectrumRiseSpeed : SpectrumDecaySpeed;
                const float NewPercent = FMath::FInterpConstantTo(CurrentValue, TargetValue, Delta, InterpSpeed);
                Bar->SetPercent(FMath::Clamp(NewPercent, 0.0f, 1.0f));
        }
}

void UStateWidget::UpdateSpectrumFromAnalyzer()
{
        if (!AudioAnalyzer.IsValid())
                return;

        TArray<float> LatestValues;
        if (!AudioAnalyzer->FetchSpectrum(LatestValues))
                return;

        if (LatestValues.Num() != SpectrumDisplayValues.Num())
                SpectrumDisplayValues = LatestValues;
        else
        {
                for (int32 Index = 0; Index < SpectrumDisplayValues.Num(); ++Index)
                        SpectrumDisplayValues[Index] = FMath::Lerp(SpectrumDisplayValues[Index], LatestValues[Index], SpectrumSmoothing);
        }
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

