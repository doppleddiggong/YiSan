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
class FJsonObject;

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

    static FString Ask = FString("/ask");
    static FString TestSTT = FString("/test/stt");
    static FString TestTTS = FString("/test/tts");
    static FString TestGPT = FString("/test/gpt");
}

// --- GPT 공간 컨텍스트 ---

USTRUCT(BlueprintType)
struct FGPTSpatialContextLocation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString name;

    UPROPERTY(BlueprintReadWrite)
    float x = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float y = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float z = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bValid = false;

    void Reset();
    void Set(const FString& InName, const FVector& InPosition);
    bool IsValid() const;
    TSharedPtr<FJsonObject> ToJsonObject() const;
};

USTRUCT(BlueprintType)
struct FGPTSpatialContextNearbyBuilding
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString name;

    UPROPERTY(BlueprintReadWrite)
    float distance = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bValid = false;

    void Reset();
    void Set(const FString& InName, float InDistanceMeters);
    bool IsValid() const;
    TSharedPtr<FJsonObject> ToJsonObject() const;
};

USTRUCT(BlueprintType)
struct FGPTSpatialContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FGPTSpatialContextLocation current_location;

    UPROPERTY(BlueprintReadWrite)
    FGPTSpatialContextLocation focused_object;

    UPROPERTY(BlueprintReadWrite)
    TArray<FGPTSpatialContextNearbyBuilding> nearby_buildings;

    void Reset();
    bool HasAnyData() const;
    void AddNearbyBuilding(const FGPTSpatialContextNearbyBuilding& InBuilding);
    TSharedPtr<FJsonObject> ToJsonObject() const;
};

struct FResponseHealth;
struct FResponseAsk;
struct FResponseTestSTT;
struct FResponseTestTTS;
struct FResponseTestGPT;

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

// --- Ask Endpoint (Integrated Pipeline: STT -> GPT -> TTS) ---

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

DECLARE_DELEGATE_TwoParams( FResponseTestSTTDelegate, FResponseTestSTT&, bool );
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
struct FRequestTestTTS
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

DECLARE_DELEGATE_TwoParams( FResponseTestTTSDelegate, FResponseTestTTS&, bool );
USTRUCT(BlueprintType)
struct FResponseTestTTS
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TArray<uint8> audio_data;

    void SetFromHttpResponse(const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response);
    void PrintData();
};


USTRUCT(BlueprintType)
struct FRequestTestGPT
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString text;
    UPROPERTY(BlueprintReadWrite)
    FString user_query;

    UPROPERTY(BlueprintReadWrite)
    FGPTSpatialContext context;

    bool ToJsonString(FString& OutJson) const;
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
