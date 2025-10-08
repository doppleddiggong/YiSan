// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AYiSanGameMode.generated.h"

/**
 * @brief 간단한 서드 퍼슨 게임을 위한 기본 게임 모드.
 *
 * 프로젝트 공통 게임모드 기능을 파생 클래스가 확장할 수 있도록 최소 구현만 제공한다.
 */
UCLASS(abstract)
class AYiSanGameMode : public AGameModeBase
{
        GENERATED_BODY()
public:

        /** @brief GameMode 기본 생성자. */
        AYiSanGameMode();
};



