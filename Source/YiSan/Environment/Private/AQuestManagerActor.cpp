// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AQuestManagerActor.h"

#include "GameLogging.h"
#include "Macro.h"
#include "UBroadcastManager.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

/**
 * @file AQuestManagerActor.cpp
 * @brief AQuestManagerActor의 동작을 구현합니다.
 */

/**
 * @brief 주어진 월드 컨텍스트에서 퀘스트 매니저를 찾는 헬퍼입니다.
 * @param WorldContextObject 대상 월드를 찾을 때 사용하는 객체입니다.
 * @return 퀘스트 매니저 인스턴스를 반환하며 없으면 @c nullptr을 반환합니다.
 */
AQuestManagerActor* AQuestManagerActor::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
        return nullptr;

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
        return nullptr;

    for (TActorIterator<AQuestManagerActor> It(World); It; ++It)
        return *It;

    return nullptr;
}

/** @brief 퀘스트 매니저의 복제 기본값을 설정합니다. */
AQuestManagerActor::AQuestManagerActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;
    
    SetReplicateMovement(false);
}

/** @brief 퀘스트 진행을 초기화하고 초기 목표를 클라이언트에 전달합니다. */
void AQuestManagerActor::StartQuest()
{
    if (HasAuthority())
    {
        if (QuestList.Num() > 0)
        {
            QuestIndex = 0;
            QuestTarget = QuestList[QuestIndex];
        }
        else
        {
            QuestIndex = INDEX_NONE;
            QuestTarget = EBuildingType::None;
        }

        SendUpdateQuest();
    }
}


/** @brief 복제를 위한 퀘스트 속성을 선언합니다. */
void AQuestManagerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AQuestManagerActor, QuestIndex);
    DOREPLIFETIME(AQuestManagerActor, QuestTarget);
}
    
/** @brief 퀘스트 인덱스가 복제될 때 로그를 남깁니다. */
void AQuestManagerActor::OnRep_CurQuestIndex()
{
    PRINTLOG(TEXT("OnRep_CurQuestIndex : %d"), QuestIndex);
}

/** @brief 퀘스트 목표 복제를 처리하고 리스너에 통보합니다. */
void AQuestManagerActor::OnRep_CurQuestTarget()
{
    PRINTLOG(TEXT("OnRep_CurQuestTarget : %d"), QuestTarget);
    SendUpdateQuest();
}

/** @brief 건물 접촉 이벤트를 처리하는 서버 RPC입니다. */
void AQuestManagerActor::ServerRPC_ContactBuilding_Implementation(EBuildingType InType)
{
    ContactBuilding(InType);
}

/** @brief 클라이언트 또는 서버 권한에서 전달되는 접촉 이벤트 진입점입니다. */
void AQuestManagerActor::OnContactBuilding(EBuildingType InType)
{
    if (HasAuthority())
        ContactBuilding(InType);
    else
        ServerRPC_ContactBuilding(InType);
}

/** @brief 퀘스트 진행도를 갱신하고 클라이언트에 통지합니다. */
void AQuestManagerActor::NextQuest()
{
    if (!HasAuthority())
        return;

    if (!QuestList.IsValidIndex(QuestIndex))
    {
        PRINTLOG(TEXT("[QUEST] NextQuest called with invalid index"));
        QuestTarget = EBuildingType::None;
        SendUpdateQuest();
        return;
    }

    ++QuestIndex;
    QuestTarget = QuestList.IsValidIndex(QuestIndex) ? QuestList[QuestIndex] : QuestTarget = EBuildingType::None;
    SendUpdateQuest();
}

/** @brief 접촉한 건물이 활성 퀘스트 목표와 일치하는지 검증합니다. */
void AQuestManagerActor::ContactBuilding(EBuildingType InType)
{
    if (!HasAuthority())
        return;

    if (!HasActiveQuest())
    {
        PRINTLOG(TEXT("[QUEST] Contact ignored. No active quest."));
        return;
    }

    if (InType != QuestTarget)
    {
        PRINTLOG(TEXT("[QUEST] Contact %s ignored. Current target is %s"),
            *ENUM_TO_NAME(EBuildingType, InType), *ENUM_TO_NAME(EBuildingType, QuestTarget));
        return;
    }

    PRINTLOG(TEXT("[QUEST] Contact confirmed for %s"), *ENUM_TO_NAME(EBuildingType, QuestTarget));
    NextQuest();
}

/** @brief 퀘스트 목표 변경 사항을 UI 계층에 브로드캐스트합니다. */
void AQuestManagerActor::SendUpdateQuest()
{
    if (auto BM =  UBroadcastManager::Get(this) )
        BM->SendUpdateQuest(QuestTarget);
}