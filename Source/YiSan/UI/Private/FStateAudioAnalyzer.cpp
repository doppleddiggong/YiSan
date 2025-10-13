// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "FStateAudioAnalyzer.h"
#include "AudioCapture.h"
#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"

namespace
{
    /** @brief 원주율(Pi) 상수입니다. */
    constexpr float Pi = 3.14159265358979323846f;
}

/**
 * @brief 기본 생성자입니다.
 */
FStateAudioAnalyzer::FStateAudioAnalyzer() = default;

/**
 * @brief 소멸자입니다.
 */
FStateAudioAnalyzer::~FStateAudioAnalyzer()
{
    Stop();
}

/**
 * @brief 오디오 캡처 및 분석을 시작합니다.
 */
bool FStateAudioAnalyzer::Start(int32 InBarCount)
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

/**
 * @brief 오디오 캡처 및 분석을 중지합니다.
 */
void FStateAudioAnalyzer::Stop()
{
    if (AudioCapture)
    {
        AudioCapture->StopStream();
        AudioCapture->CloseStream();
    }
}

/**
 * @brief 가장 최근에 계산된 스펙트럼 데이터를 가져옵니다.
 */
bool FStateAudioAnalyzer::FetchSpectrum(TArray<float>& OutSpectrum)
{
    FScopeLock Lock(&SpectrumGuard);
    if (!bPending)
        return false;

    OutSpectrum = PendingSpectrum;
    bPending = false;
    return true;
}

/**
 * @brief 캡처 스트림에서 들어오는 오디오 데이터를 처리하는 콜백 함수입니다.
 */
void FStateAudioAnalyzer::HandleAudio(const float* InAudio, int32 InNumFrames, int32 InNumChannels, int32 InSampleRate)
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

/**
 * @brief 스펙트럼 누출을 줄이기 위해 FFT 입력에 Hann 창을 적용합니다.
 */
void FStateAudioAnalyzer::ApplyHannWindow()
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

/**
 * @brief FFT 입력 데이터에서 주파수 스펙트럼을 계산합니다.
 */
void FStateAudioAnalyzer::ComputeSpectrum(int32 SampleRate)
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