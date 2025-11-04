// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "GameFramework/Actor.h"
#include "AQuestManagerActor.generated.h"

/**
 * @file AQuestManagerActor.h
 * @brief AQuestManagerActor 클래스를 선언합니다.
 */


/**
 * @brief 서버 주도형 퀘스트 진행 상태를 관리하는 액터입니다.
 * @details 기존 UQuestManager 서브시스템을 대체하며, 서버에서 목표 정보를 관리하고 복제로 동기화합니다.
 */
UCLASS()
class YISAN_API AQuestManagerActor : public AActor
{
    GENERATED_BODY()

public:
    AQuestManagerActor();

    static AQuestManagerActor* Get(const UObject* WorldContextObject);

    void StartQuest();
    void SendUpdateQuest();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category="Quest")
    EBuildingType GetCurrentTarget() const { return QuestTarget; }

    UFUNCTION(BlueprintCallable, Category="Quest")
    bool HasActiveQuest() const { return QuestTarget != EBuildingType::None; }
    void OnContactBuilding(EBuildingType InType);


protected:
    UFUNCTION(Server, Reliable)
    void ServerRPC_ContactBuilding(EBuildingType InType);

private:
    void NextQuest();
    void ContactBuilding(EBuildingType InType);

    UFUNCTION()
    void OnRep_CurQuestIndex();

    UFUNCTION()
    void OnRep_CurQuestTarget();

private:
    UPROPERTY(EditDefaultsOnly, Category="Quest")
    TArray<EBuildingType> QuestList = {
        EBuildingType::Sinpungnu,
        EBuildingType::Uhwagwan,
        EBuildingType::Jwaikmun,
        EBuildingType::Bongsudang,
        EBuildingType::Yeomingak,
        EBuildingType::Byeolju,
    };
    
    UPROPERTY(ReplicatedUsing=OnRep_CurQuestIndex)
    int32 QuestIndex = 0;

    UPROPERTY(ReplicatedUsing=OnRep_CurQuestTarget)
    EBuildingType QuestTarget = EBuildingType::Sinpungnu;
};
