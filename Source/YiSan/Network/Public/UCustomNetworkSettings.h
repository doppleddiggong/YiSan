// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/// @file UCustomNetworkSettings.h
/// @brief 서버 환경별 네트워크 설정을 노출하는 DeveloperSettings를 선언합니다.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UCustomNetworkSettings.generated.h"

/// @brief 서버 환경(개발/스테이징/라이브)을 나타내는 열거형입니다.
UENUM(BlueprintType)
enum class EServerMode : uint8
{
    Dev,   ///< 개발 서버 모드입니다.
    Stage, ///< 스테이징 서버 모드입니다.
    Live   ///< 라이브 서버 모드입니다.
};

/// @brief 서버 접속 정보를 담는 설정 구조체입니다.
USTRUCT(BlueprintType)
struct FServerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, config, Category="Network")
    FString BaseUrl;

    UPROPERTY(EditAnywhere, config, Category="Network")
    int32 Port = 0;

    UPROPERTY(EditAnywhere, config, Category="Network")
    FString WebSocketUrl;

    /// @brief 엔드포인트를 결합한 전체 URL을 반환합니다.
    /// @param Endpoint [in] 호출할 상대 경로입니다.
    FString GetFullUrl(const FString& Endpoint) const
    {
        if (Port == 443 || Port == 0)
        {
            return FString::Printf(TEXT("%s%s"), *BaseUrl, *Endpoint);
        }

        return FString::Printf(TEXT("%s:%d%s"), *BaseUrl, Port, *Endpoint);
    }
};


UCLASS(config=MyNetwork, defaultconfig, meta=(DisplayName="Network Selector Setting"))
class YISAN_API UCustomNetworkSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /// @brief 명령줄 또는 설정으로 선택된 서버 모드를 반환합니다.
    static EServerMode GetCurrentServerMode();

    UPROPERTY(config, EditAnywhere, Category="Network")
    EServerMode DefaultMode = EServerMode::Dev;

    UPROPERTY(config, EditAnywhere, Category="Network")
    FServerConfig DevConfig;

    UPROPERTY(config, EditAnywhere, Category="Network")
    FServerConfig StageConfig;

    UPROPERTY(config, EditAnywhere, Category="Network")
    FServerConfig LiveConfig;

    /// @brief 서버 모드에 해당하는 설정을 반환합니다.
    /// @param Mode [in] 조회할 서버 모드입니다.
    /// @return 선택한 모드의 서버 구성입니다.
    const FServerConfig& GetConfig(EServerMode Mode) const;
};
