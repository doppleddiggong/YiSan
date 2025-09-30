// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

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
}

namespace RequestAPI
{
    static FString Health = FString("/health");
    static FString HelpChat = FString("/help_chat");
}

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
