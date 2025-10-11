// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file NetworkData.h
 * @brief NetworkData 구조체 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "UCustomNetworkSettings.h"
#include "Templates/SharedPointer.h"
#include "NetworkData.generated.h"

class IHttpResponse;

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
    static FString HelpChat = FString("/help_chat");
    static FString STT = FString("/stt");
    static FString FishTTSSimple = FString("/fish/tts-simple");
    static FString TTS = FString("/tts");
    static FString Ask = FString("/ask");
    static FString TestGPT = FString("/test/gpt");
    static FString TestTTS = FString("/test/tts");
}

USTRUCT(BlueprintType)
struct FRequestTTS
{
    GENERATED_BODY()

    UPROPERTY()
    FString text;
    
    UPROPERTY()
    FString reference_index;

    UPROPERTY()
    bool return_audio = false;
};

DECLARE_DELEGATE_TwoParams( FResponseHealthDelegate, FResponseHealth&, bool );
USTRUCT(BlueprintType)
struct FResponseHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 status = 0;

    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);

    void PrintData();
};

DECLARE_DELEGATE_TwoParams( FResponseHelpChatDelegate, FResponseHelpChat&, bool );
USTRUCT(BlueprintType)
struct FRequestHelpChat
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString question;
};

USTRUCT(BlueprintType)
struct FResponseHelpChat
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ai_message;

    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
    void PrintData();
};

DECLARE_DELEGATE_TwoParams( FResponseSTTDelegate, FResponseSTT&, bool );
USTRUCT(BlueprintType)
struct FResponseSTT
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString text;

    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
    void PrintData();
};

USTRUCT(BlueprintType)
struct FRequestFishTTSSimple
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString text;
};

DECLARE_DELEGATE_TwoParams( FResponseTTSDelegate, FResponseTTS&, bool );
USTRUCT(BlueprintType)
struct FResponseTTS
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString message;
    UPROPERTY(BlueprintReadWrite)
    FString file_path;
    UPROPERTY(BlueprintReadWrite)
    FString requested_text;
    UPROPERTY(BlueprintReadWrite)
    float generation_time;
    UPROPERTY(BlueprintReadWrite)
    FString version;

    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
    void PrintData();
};

// --- Test Endpoints ---

USTRUCT(BlueprintType)
struct FRequestTestGPT
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString text;
};

DECLARE_DELEGATE_TwoParams( FResponseTestGPTDelegate, FResponseTestGPT&, bool );
USTRUCT(BlueprintType)
struct FResponseTestGPT
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString response;

    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
    void PrintData();
};

USTRUCT(BlueprintType)
struct FRequestTestTTS
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString text;

    UPROPERTY(BlueprintReadWrite)
    float speaking_rate = 0.88f;

    UPROPERTY(BlueprintReadWrite)
    float pitch = -3.0f;
};

DECLARE_DELEGATE_TwoParams( FResponseTestTTSDelegate, TArray<uint8>&, bool );
USTRUCT(BlueprintType)
struct FResponseTestTTS
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TArray<uint8> audio_data;

    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
    void PrintData();
};