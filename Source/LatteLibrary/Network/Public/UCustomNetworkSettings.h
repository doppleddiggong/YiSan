// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UCustomNetworkSettings.generated.h"

UENUM(BlueprintType)
enum class EServerMode : uint8
{
	Dev,
	Stage,
	Live
};

USTRUCT(BlueprintType)
struct FServerConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, config, Category="Network")
	FString BaseUrl;

	UPROPERTY(EditAnywhere, config, Category="Network")
	int32 Port = 0;

	FString GetFullUrl(const FString& Endpoint) const
	{
		return FString::Printf(TEXT("%s:%d%s"), *BaseUrl, Port, *Endpoint);
	}
};


UCLASS(config=MyNetwork, defaultconfig, meta=(DisplayName="Network Selector Setting"))
class LATTELIBRARY_API UCustomNetworkSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static EServerMode GetCurrentServerMode();

	UPROPERTY(config, EditAnywhere, Category="Network")
	EServerMode DefaultMode = EServerMode::Dev;

	UPROPERTY(config, EditAnywhere, Category="Network")
	FServerConfig DevConfig;

	UPROPERTY(config, EditAnywhere, Category="Network")
	FServerConfig StageConfig;

	UPROPERTY(config, EditAnywhere, Category="Network")
	FServerConfig LiveConfig;

	const FServerConfig& GetConfig(EServerMode Mode) const;
};
