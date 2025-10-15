// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UQuestManager.generated.h"

/**
 * @class UQuestManager
 * @brief Quest progression subsystem that manages ordered building targets and reacts to broadcast events.
 */
UCLASS()
class YISAN_API UQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    DEFINE_SUBSYSTEM_GETTER_INLINE(UQuestManager);

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    FORCEINLINE EBuildingType GetCurrentTarget() const
    {
        return CurTarget;
    }

    void InitSystem();
    
private:
    UFUNCTION()
    void OnContactBuilding(EBuildingType InType);
    
private:
    TArray<EBuildingType> QuestList;
    int32 CurQuestIndex = INDEX_NONE;
    EBuildingType CurTarget = EBuildingType::None;

    UPROPERTY()
    TObjectPtr<class UBroadcastManager> BroadcastManager;
};
