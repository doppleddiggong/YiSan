// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "FGPTContext.h"
#include "Components/ActorComponent.h"
#include "UGPTContextSystem.generated.h"

/// @file UGPTContextSystem.h
/// @brief 환경 정보를 반영한 프롬프트를 구성하는 GPT 컨텍스트 컴포넌트를 선언합니다.
/// @brief 소유한 플레이어를 위해 GPT 프롬프트 컨텍스트를 조합하는 액터 컴포넌트입니다.
///
/// @details 이 컴포넌트는 카메라 시선, 주변 건물, 캐시된 게임 데이터를 점검하여 구조화된
/// FGPTContext 페이로드를 생성합니다. 생성된 컨텍스트는 음성 대화 워크플로에서 사용되어
/// GPT 서비스가 위치 정보를 반영한 응답을 제공할 수 있도록 돕습니다.
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class YISAN_API UGPTContextSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    /// @brief 컨텍스트를 지속적으로 추적할 수 있도록 틱을 활성화하는 기본 생성자입니다.
    UGPTContextSystem();

public:
    /// @brief 현재 시선 건물을 갱신하는 매 프레임 업데이트입니다.
    /// @param DeltaTime [in] 프레임 델타 시간(초)입니다.
    /// @param TickType [in] 이 컴포넌트에 적용된 틱 유형입니다.
    /// @param ThisTickFunction [in] 엔진이 제공하는 틱 메타데이터입니다.
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    /// @brief 소유 플레이어 참조로 컴포넌트를 초기화합니다.
    /// @param InOwner [in] 카메라 정보와 데이터 매니저를 제공하는 플레이어 액터입니다.
    void InitSystem(class APlayerActor* InOwner);

    /// @brief 대화 시스템에서 사용하는 GPT 컨텍스트 스냅샷을 구축합니다.
    /// @return 플레이어, 시선 대상, 주변 건물을 담은 FGPTContext입니다.
    FGPTContext GetGPTContext() const;

private:
    /// @brief 현재 시선에 포착된 건물을 판별하기 위해 카메라 트레이스를 수행합니다.
    void CheckBuildingInView();
    void FindNearestBuilding();

private:
    UPROPERTY()
    TObjectPtr<class APlayerActor> Owner; ///< 위치와 카메라 데이터를 조회하는 소유 플레이어입니다.

    UPROPERTY()
    TObjectPtr<class UBroadcastManager> BroadcastManager; ///< UI와 시스템에 알림을 전달하는 브로드캐스트 매니저입니다.

    UPROPERTY()
    TOptional<EBuildingType> FocusBuildingType; ///< 플레이어 시선에 있는 건물 유형을 캐시한 값입니다.

    UPROPERTY()
    TWeakObjectPtr<class ABuilding> NearestBuilding;

    float TimeSinceLastCheck = 0.0f;
};
