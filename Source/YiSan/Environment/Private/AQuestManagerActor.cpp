// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AQuestManagerActor.h"

#include "GameLogging.h"
#include "Macro.h"
#include "UBroadcastManager.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

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

AQuestManagerActor::AQuestManagerActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;
    
    SetReplicateMovement(false);
}

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


void AQuestManagerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AQuestManagerActor, QuestIndex);
    DOREPLIFETIME(AQuestManagerActor, QuestTarget);
}
    
void AQuestManagerActor::OnRep_CurQuestIndex()
{
    PRINTLOG(TEXT("OnRep_CurQuestIndex : %d"), QuestIndex);
}

void AQuestManagerActor::OnRep_CurQuestTarget()
{
    PRINTLOG(TEXT("OnRep_CurQuestTarget : %d"), QuestTarget);
    SendUpdateQuest();
}

void AQuestManagerActor::ServerRPC_ContactBuilding_Implementation(EBuildingType InType)
{
    ContactBuilding(InType);
}

void AQuestManagerActor::OnContactBuilding(EBuildingType InType)
{
    if (HasAuthority())
        ContactBuilding(InType);
    else
        ServerRPC_ContactBuilding(InType);
}

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

void AQuestManagerActor::SendUpdateQuest()
{
    if (auto BM =  UBroadcastManager::Get(this) )
        BM->SendUpdateQuest(QuestTarget);
}