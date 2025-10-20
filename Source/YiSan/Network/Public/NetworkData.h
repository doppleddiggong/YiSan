// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/// @file NetworkData.h
/// @brief 네트워크 요청과 응답에 사용되는 구조체 및 설정을 정의합니다.
#pragma once

#include "CoreMinimal.h"
#include "FGPTContext.h"
#include "UCustomNetworkSettings.h"
#include "Templates/SharedPointer.h"
#include "NetworkData.generated.h"

// =================================================================================
// Network Configuration
// 서버 주소와 포트를 설정합니다.
// =================================================================================
namespace NetworkConfig
{
    /// @brief 현재 서버 모드에 맞는 HTTP 엔드포인트 URL을 생성합니다.
    /// @param Endpoint [in] 호출할 상대 경로입니다.
    /// @return 모드와 포트가 적용된 전체 URL입니다.
    static FString GetFullUrl(const FString& Endpoint)
    {
        const EServerMode Mode = UCustomNetworkSettings::GetCurrentServerMode();
        const FServerConfig& Config = GetDefault<UCustomNetworkSettings>()->GetConfig(Mode);
        return Config.GetFullUrl(Endpoint);
    }

    /// @brief 현재 서버 모드에서 사용할 WebSocket 주소를 반환합니다.
    /// @return ws(s):// 형식의 소켓 접속 URL입니다.
    static FString GetSocketURL()
    {
        const EServerMode Mode = UCustomNetworkSettings::GetCurrentServerMode();
        const FServerConfig& Config = GetDefault<UCustomNetworkSettings>()->GetConfig(Mode);
        return Config.WebSocketUrl;
    }
}

namespace RequestAPI
{
    /// @brief 서버 상태 확인 엔드포인트입니다.
    static FString Health = FString("/health");

    /// @brief 음성-텍스트-음성을 통합 처리하는 ASK 엔드포인트입니다.
    static FString ASK = FString("/ask");

    /// @brief 음성 인식 엔드포인트입니다.
    static FString STT = FString("/stt");

    /// @brief 텍스트 음성 합성 엔드포인트입니다.
    static FString TTS = FString("/tts");

    /// @brief 텍스트 기반 GPT 질의 엔드포인트입니다.
    static FString GPT = FString("/gpt");
}


/// @brief 헬스 체크 응답을 전달하는 델리게이트입니다.
DECLARE_DELEGATE_TwoParams(FResponseHealthDelegate, FResponseHealth&, bool);
USTRUCT(BlueprintType)
struct FResponseHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 status = 0;

    /// @brief HTTP 응답을 파싱해 상태 정보를 갱신합니다.
    void SetFromHttpResponse(const TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe>& Response);

    /// @brief 디버그 로그에 응답 내용을 출력합니다.
    void PrintData();
};

// --- Ask Endpoint (Integrated Pipeline: STT -> GPT -> TTS) ---
USTRUCT(BlueprintType)
struct FRequestASK
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FGPTContext context;

    /// @brief 구조체를 JSON 문자열로 직렬화합니다.
    bool ToJsonString(FString& OutJson) const;
};


USTRUCT(BlueprintType)
struct FRequestGPT
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString user_query;

    UPROPERTY(BlueprintReadWrite)
    FGPTContext context;

    /// @brief 구조체를 JSON 문자열로 변환합니다.
    bool ToJsonString(FString& OutJson) const;
};

/// @brief ASK/GPT 응답을 전달하는 델리게이트입니다.
DECLARE_DELEGATE_TwoParams(FResponseAskDelegate, FResponseAsk&, bool);
USTRUCT(BlueprintType)
struct FResponseAsk
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString transcribed_text;

    UPROPERTY(BlueprintReadWrite)
    FString gpt_response_text;

    UPROPERTY(BlueprintReadWrite)
    TArray<uint8> audio_data;

    /// @brief HTTP 응답을 해석해 필드 값을 채웁니다.
    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
    /// @brief 응답 데이터를 로그에 출력합니다.
    void PrintData();
};

/// @brief 음성 인식 결과를 알리는 델리게이트입니다.
DECLARE_DELEGATE_TwoParams(FResponseSTTDelegate, FResponseTestSTT&, bool);
USTRUCT(BlueprintType)
struct FResponseTestSTT
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString text;

    /// @brief HTTP 응답을 파싱해 텍스트를 채웁니다.
    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
    /// @brief 결과 텍스트를 로그에 출력합니다.
    void PrintData();
};

// --- Test Endpoints ---

USTRUCT(BlueprintType)
struct FRequestTTS
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString text;

    UPROPERTY(BlueprintReadWrite)
    float speaking_rate = 0.88f;

    UPROPERTY(BlueprintReadWrite)
    float pitch = -3.0f;

    UPROPERTY(BlueprintReadWrite)
    FString voice_name = TEXT("ko-KR-Wavenet-D");
};

/// @brief 음성 합성 응답을 전달하는 델리게이트입니다.
DECLARE_DELEGATE_TwoParams(FResponseTTSDelegate, FResponseTTS&, bool);
USTRUCT(BlueprintType)
struct FResponseTTS
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TArray<uint8> audio_data;

    /// @brief HTTP 응답 바디에서 오디오 데이터를 추출합니다.
    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
    /// @brief 오디오 데이터 요약을 로그로 출력합니다.
    void PrintData();
};



/// @brief GPT 텍스트 응답을 전달하는 델리게이트입니다.
DECLARE_DELEGATE_TwoParams(FResponseGPTDelegate, FResponseGPT&, bool);
USTRUCT(BlueprintType)
struct FResponseGPT
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString response;

    /// @brief GPT 응답 본문을 파싱합니다.
    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
    /// @brief 응답 메시지를 로그에 출력합니다.
    void PrintData();
};
