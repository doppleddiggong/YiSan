// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AYiSanGameMode.generated.h"

/// @file AYiSanGameMode.h
/// @brief YiSan 프로젝트의 공통 게임 모드 베이스 클래스를 선언합니다.

/// @brief 프로젝트 전반에서 공유하는 기본 게임 모드 추상 클래스입니다.
UCLASS(abstract)
class AYiSanGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    virtual void PostLogin(APlayerController* NewPlayer) override;
};
