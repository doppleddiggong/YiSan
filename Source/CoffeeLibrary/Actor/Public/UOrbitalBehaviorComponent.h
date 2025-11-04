// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UOrbitalBehaviorComponent.h
 * @brief UOrbitalBehaviorComponent 클래스를 선언합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UOrbitalBehaviorComponent.generated.h"

/**
 * @brief 지정된 타겟을 중심으로 공전하며 움직이는 보조 오브젝트 컴포넌트.
 *
 * 타겟 추적, 공전 각도, 바운스 등의 움직임을 한 번에 제어한다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COFFEELIBRARY_API UOrbitalBehaviorComponent : public UActorComponent
{
        GENERATED_BODY()

public:
        /** @brief 기본 속성값을 초기화합니다. */
        UOrbitalBehaviorComponent();

        /** @brief 타겟 중심(앵커) 보간 속도. 0이면 즉시 스냅. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target Float|Follow", meta=(ClampMin="0.0"))
        float AnchorLerpSpeed = 3.f;

        /** @brief 등속 추적 사용 여부. true면 UnitsPerSec 기반으로 이동한다. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target Float|Follow")
        bool bUseConstantFollow = false;

        /** @brief 등속 추적 속도(cm/s). */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target Float|Follow", meta=(EditCondition="bUseConstantFollow", ClampMin="0.0"))
        float AnchorFollowUnitsPerSec = 800.f;

        /** @brief 공전 기준이 되는 타겟(중심점). */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target Float|Targets")
        AActor* TargetActor = nullptr;

        /** @brief 바라볼 대상. 없으면 자체 회전을 유지한다. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target Float|Targets")
        AActor* PlayerActor = nullptr;

        /** @brief 공전 반경(cm). */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target Float|Orbit", meta=(ClampMin="0.0"))
        float OrbitRadius = 200.f;

        /** @brief 타겟 중심에서의 높이 오프셋. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target Float|Orbit")
        float HeightOffset = 0.f;

        /** @brief 공전 각속도(도/초). */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target Float|Orbit")
        float IdleYawSpeedDegPerSec = 90.f;

        /** @brief 상하 바운스 진폭(cm). */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target Float|Vertical")
        float BobAmplitude = 30.f;

        /** @brief 상하 바운스 빈도(Hz). */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target Float|Vertical", meta=(ClampMin="0.0"))
        float BobSpeed = 1.2f;

        /** @brief 위치 이동 시 충돌 스윕을 수행할지 여부. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target Float|Movement")
        bool bSweepMovement = false;

        /** @brief 유효한 플레이어가 있을 때만 Yaw 회전을 맞추는지 여부. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target Float|Rotation")
        bool bFacePlayer = true;

        /** @brief 공전 타겟을 교체한다. */
        UFUNCTION(BlueprintCallable, Category="Target Float|Follow")
        void SetTargetActor(AActor* NewTarget, bool bSnapAnchor = false, bool bPreserveOrbitPhase = true);

protected:
        /** @brief 컴포넌트 활성화 시 초기 앵커 위치를 기록한다. */
        virtual void BeginPlay() override;
        /** @brief 매 프레임 공전 로직을 처리한다. */
        virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
        // 내부 상태
        float   AngleDeg  = 0.f;
        float   TimeAcc   = 0.f;
        FVector AnchorLoc = FVector::ZeroVector;

        /** @brief 타겟 추적 및 위치 보간을 처리하는 내부 함수입니다. */
        void Tick_TargetFloat(float DeltaTime);
};
