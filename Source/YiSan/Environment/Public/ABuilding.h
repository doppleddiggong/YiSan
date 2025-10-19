// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "GameFramework/Actor.h"
#include "ABuilding.generated.h"

/// @file ABuilding.h
/// @brief 건물 방문 이벤트를 브로드캐스트하는 액터를 선언합니다.

/// @brief 플레이어 접촉을 감지하고 퀘스트 진행을 알리는 건물 액터입니다.
UCLASS()
class YISAN_API ABuilding : public AActor
{
    GENERATED_BODY()

public:
    /// @brief 컴포넌트를 초기화하는 기본 생성자입니다.
    ABuilding();

protected:
    /// @brief 브로드캐스트 구독을 설정합니다.
    virtual void BeginPlay() override;

    /// @brief 퀘스트 목표가 갱신되었을 때 빛 기둥 표시를 조정합니다.
    /// @param InBuildingType [in] 현재 목표 건물 유형입니다.
    UFUNCTION()
    void OnUpdateQuest(EBuildingType InBuildingType);

    /// @brief 플레이어가 감지 범위에 진입했을 때 호출됩니다.
    /// @param OverlappedComp [in] 감지 콜리전 컴포넌트입니다.
    /// @param OtherActor [in] 감지된 액터입니다.
    /// @param OtherComp [in] 감지된 액터의 컴포넌트입니다.
    /// @param OtherBodyIndex [in] 겹친 바디 인덱스입니다.
    /// @param bFromSweep [in] 스윕 결과인지 여부입니다.
    /// @param SweepResult [in] 충돌 결과 정보입니다.
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingType BuildingType = EBuildingType::Yeomingak;

    UPROPERTY(EditAnywhere)
    class UBoxComponent* boxcomp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNextTargetBuilding = false;

protected:
    /// @brief 퀘스트 진행 상태를 표시하는 빛 기둥 메시입니다.
    UPROPERTY(VisibleAnywhere, Category = "Building")
    TObjectPtr<class UStaticMeshComponent> LightPillarMesh;

    UPROPERTY()
    TObjectPtr<class UBroadcastManager> BroadcastManager;
};
