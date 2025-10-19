// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AMassActor.generated.h"

/// @file AMassActor.h
/// @brief Mass Crowd 연동 실험용 캐릭터 액터를 선언합니다.

/// @brief Mass 시스템과 상호작용하기 위한 테스트 캐릭터입니다.
UCLASS()
class YISAN_API AMassActor : public ACharacter
{
    GENERATED_BODY()

public:
    /// @brief 기본 이동 컴포넌트를 구성하는 생성자입니다.
    AMassActor();

protected:
    /// @brief 스폰 시 초기화를 수행합니다.
    virtual void BeginPlay() override;

public:
    /// @brief 프레임마다 Mass 연동 로직을 업데이트합니다.
    /// @param DeltaTime [in] 프레임 델타 시간입니다.
    virtual void Tick(float DeltaTime) override;

    /// @brief 입력 컴포넌트를 바인딩합니다.
    /// @param PlayerInputComponent [in] 입력 매핑을 구성할 컴포넌트입니다.
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
