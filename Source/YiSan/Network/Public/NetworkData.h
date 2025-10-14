// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file NetworkData.h
 * @brief NetworkData 구조체 선언에 대한 Doxygen 주석을 제공합니다.
 */
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
    static FString GetFullUrl(const FString& Endpoint)
    {
        const EServerMode Mode = UCustomNetworkSettings::GetCurrentServerMode();
        const FServerConfig& Config = GetDefault<UCustomNetworkSettings>()->GetConfig(Mode);
        return Config.GetFullUrl(Endpoint);
    }

    static FString GetSocketURL()
    {
        const EServerMode Mode = UCustomNetworkSettings::GetCurrentServerMode();
        const FServerConfig& Config = GetDefault<UCustomNetworkSettings>()->GetConfig(Mode);
        return Config.WebSocketUrl;
    }
    
}

namespace RequestAPI
{
    static FString Health = FString("/health");

    static FString ASK = FString("/ask");
    static FString STT = FString("/stt");
    static FString TTS = FString("/tts");
    static FString GPT = FString("/gpt");
}


DECLARE_DELEGATE_TwoParams( FResponseHealthDelegate, FResponseHealth&, bool );
USTRUCT(BlueprintType)
struct FResponseHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 status = 0;

    void SetFromHttpResponse(const TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe>& Response);

    void PrintData();
};

// --- Ask Endpoint (Integrated Pipeline: STT -> GPT -> TTS) ---
USTRUCT(BlueprintType)
struct FRequestASK
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FGPTContext context;

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

    bool ToJsonString(FString& OutJson) const;
};

DECLARE_DELEGATE_TwoParams( FResponseAskDelegate, FResponseAsk&, bool );
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

    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
    void PrintData();
};

DECLARE_DELEGATE_TwoParams( FResponseSTTDelegate, FResponseTestSTT&, bool );
USTRUCT(BlueprintType)
struct FResponseTestSTT
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString text;

    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
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

DECLARE_DELEGATE_TwoParams( FResponseTTSDelegate, FResponseTTS&, bool );
USTRUCT(BlueprintType)
struct FResponseTTS
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TArray<uint8> audio_data;

    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
    void PrintData();
};



DECLARE_DELEGATE_TwoParams( FResponseGPTDelegate, FResponseGPT&, bool );
USTRUCT(BlueprintType)
struct FResponseGPT
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString response;

    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
    void PrintData();
};
