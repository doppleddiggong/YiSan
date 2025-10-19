// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "GameFramework/Actor.h"
#include "ABuildingContainer.generated.h"

/// @file ABuildingContainer.h
/// @brief 레벨 내 건물 액터를 수집·조회하는 헬퍼 액터를 선언합니다.

/// @brief 자식 건물들을 맵에 캐시하여 빠르게 조회할 수 있도록 돕는 액터입니다.
UCLASS()
class YISAN_API ABuildingContainer : public AActor
{
    GENERATED_BODY()

public:
    /// @brief 기본 값과 편집용 속성을 초기화합니다.
    ABuildingContainer();

protected:
    /// @brief 시작 시 건물 맵을 구성합니다.
    virtual void BeginPlay() override;

public:
    /// @brief 에디터에서 하위 건물을 스캔해 맵을 갱신합니다.
    UFUNCTION(CallInEditor, Category="BuildingContainer", meta=(DevelopmentOnly, DisplayName="Collect Buildings"))
    void CollectBuildings();

    /// @brief 유형별로 건물 액터를 반환합니다.
    /// @param InBuildingType [in] 검색할 건물 유형입니다.
    /// @return 해당 유형의 건물 액터, 없으면 nullptr입니다.
    UFUNCTION(BlueprintCallable, Category="BuildingContainer")
    class ABuilding* GetBuilding(EBuildingType InBuildingType) const;

protected:
    /// @brief 현재 자식 액터 상태를 기반으로 맵을 재구성합니다.
    void RefreshBuildingMap();

protected:
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="BuildingContainer")
    TMap<EBuildingType, TObjectPtr<class ABuilding>> BuildingMap;
};
