// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UQuestManager.generated.h"

/// @file UQuestManager.h
/// @brief 건물 방문 퀘스트 진행을 관리하는 게임 인스턴스 서브시스템을 선언합니다.

/// @brief 순차적인 건물 목표를 추적하며 브로드캐스트 이벤트에 반응하는 퀘스트 매니저입니다.
UCLASS()
class YISAN_API UQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    DEFINE_SUBSYSTEM_GETTER_INLINE(UQuestManager);

    /// @brief 브로드캐스트 구독과 초기 상태를 준비합니다.
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /// @brief 등록된 델리게이트를 해제합니다.
    virtual void Deinitialize() override;

    /// @brief 현재 목표 건물 유형을 반환합니다.
    FORCEINLINE EBuildingType GetCurrentTarget() const
    {
        return CurTarget;
    }

    /// @brief 퀘스트 데이터와 브로드캐스트 연동을 초기화합니다.
    void InitSystem();

private:
    /// @brief 플레이어가 특정 건물과 접촉했을 때 호출됩니다.
    /// @param InType [in] 방문한 건물 유형입니다.
    UFUNCTION()
    void OnContactBuilding(EBuildingType InType);

private:
    TArray<EBuildingType> QuestList;
    int32 CurQuestIndex = INDEX_NONE;
    EBuildingType CurTarget = EBuildingType::None;

    UPROPERTY()
    TObjectPtr<class UBroadcastManager> BroadcastManager;
};
