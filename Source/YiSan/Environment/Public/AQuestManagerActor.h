// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "GameFramework/Actor.h"
#include "AQuestManagerActor.generated.h"


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
    
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category="Quest")
    EBuildingType GetCurrentTarget() const { return CurQuestTarget; }

    UFUNCTION(BlueprintCallable, Category="Quest")
    int32 GetCurrentQuestIndex() const { return CurQuestIndex; }

    UFUNCTION(BlueprintCallable, Category="Quest")
    bool HasActiveQuest() const { return CurQuestTarget != EBuildingType::None; }

    void StartQuest();
    void NotifyContact(EBuildingType InType);
    void BroadcastQuestUpdate();

protected:
    UFUNCTION(Server, Reliable)
    void ServerRPC_NotifyContact(EBuildingType InType);

private:
    void InitializeQuestList();
    void ResetQuestProgress();
    void AdvanceQuest();
    void HandleQuestUpdated();
    void HandleContactInternal(EBuildingType InType);

    UFUNCTION()
    void OnRep_CurQuestIndex();

    UFUNCTION()
    void OnRep_CurQuestTarget();

private:
    UPROPERTY(EditDefaultsOnly, Category="Quest")
    TArray<EBuildingType> QuestList;

    UPROPERTY(ReplicatedUsing=OnRep_CurQuestIndex)
    int32 CurQuestIndex;

    UPROPERTY(ReplicatedUsing=OnRep_CurQuestTarget)
    EBuildingType CurQuestTarget;

    UPROPERTY()
    TObjectPtr<class UBroadcastManager> BroadcastManager;

    bool bInitialized;
};
