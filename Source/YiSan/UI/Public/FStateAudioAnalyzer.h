// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"

namespace Audio
{
    class FAudioCapture;
}

/**
 * @class FStateAudioAnalyzer
 * @brief 오디오 입력을 분석하여 주파수 스펙트럼을 생성합니다.
 *
 * 이 클래스는 기본 입력 장치에서 오디오를 캡처하고, FFT(고속 푸리에 변환)를 수행하여
 * 오디오 데이터를 주파수 도메인으로 변환한 후 결과 스펙트럼 데이터를 제공합니다.
 * 게임 스레드의 끊김 현상을 방지하기 위해 별도의 스레드나 컨텍스트에서 사용하도록 설계되었습니다.
 */
class YISAN_API FStateAudioAnalyzer : public TSharedFromThis<FStateAudioAnalyzer>
{
public:
    /**
     * @brief 기본 생성자입니다.
     */
    FStateAudioAnalyzer();

    /**
     * @brief 소멸자입니다. 오디오 캡처 스트림이 중지되고 닫히도록 보장합니다.
     */
    ~FStateAudioAnalyzer();

    /**
     * @brief 오디오 캡처 및 분석을 시작합니다.
     * @param InBarCount 스펙트럼을 나눌 주파수 막대의 수입니다.
     * @return 오디오 스트림이 성공적으로 시작되면 true, 그렇지 않으면 false를 반환합니다.
     */
    bool Start(int32 InBarCount);

    /**
     * @brief 오디오 캡처 및 분석을 중지합니다.
     */
    void Stop();

    /**
     * @brief 가장 최근에 계산된 스펙트럼 데이터를 가져옵니다.
     * @param OutSpectrum 스펙트럼 데이터로 채워질 TArray입니다.
     * @return 새로운 스펙트럼 데이터를 사용할 수 있으면 true, 그렇지 않으면 false를 반환합니다.
     */
    bool FetchSpectrum(TArray<float>& OutSpectrum);

private:
    /**
     * @brief 캡처 스트림에서 들어오는 오디오 데이터를 처리하는 콜백 함수입니다.
     * @param InAudio 오디오 데이터 버퍼에 대한 포인터입니다.
     * @param InNumFrames 버퍼의 프레임 수입니다.
     * @param InNumChannels 오디오 채널 수입니다.
     * @param InSampleRate 오디오의 샘플 속도입니다.
     */
    void HandleAudio(const float* InAudio, int32 InNumFrames, int32 InNumChannels, int32 InSampleRate);

    /**
     * @brief 스펙트럼 누출을 줄이기 위해 FFT 입력에 Hann 창을 적용합니다.
     */
    void ApplyHannWindow();

    /**
     * @brief FFT 입력 데이터에서 주파수 스펙트럼을 계산합니다.
     * @param SampleRate 오디오의 샘플 속도입니다.
     */
    void ComputeSpectrum(int32 SampleRate);

private:
    /** @brief 오디오 캡처 컴포넌트입니다. */
    TUniquePtr<Audio::FAudioCapture> AudioCapture;

    /** @brief 수행할 FFT의 크기입니다. */
    int32 FFTSize = 256;

    /** @brief 스펙트럼을 나눌 막대의 수입니다. */
    int32 BarCount = 32;

    /** @brief FFT를 위한 입력 버퍼입니다. */
    TArray<float> FFTInput;

    /** @brief Hann 창 값입니다. */
    TArray<float> Window;

    /** @brief 스펙트럼의 크기입니다. */
    TArray<float> Magnitude;

    /** @brief 가장 최근에 계산된 스펙트럼 데이터입니다. */
    TArray<float> LatestSpectrum;

    /** @brief 가져올 보류 중인 스펙트럼 데이터입니다. */
    TArray<float> PendingSpectrum;

    /** @brief 보류 중인 스펙트럼 데이터에 대한 접근을 보호하기 위한 크리티컬 섹션입니다. */
    FCriticalSection SpectrumGuard;

    /** @brief 보류 중인 스펙트럼 데이터가 있는지 여부를 나타내는 플래그입니다. */
    bool bPending = false;
};