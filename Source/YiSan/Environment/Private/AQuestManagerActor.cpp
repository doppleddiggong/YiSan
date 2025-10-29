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
    : CurQuestIndex(INDEX_NONE)
    , CurQuestTarget(EBuildingType::None)
    , bInitialized(false)
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;
    
    SetReplicateMovement(false);
}

void AQuestManagerActor::BeginPlay()
{
    Super::BeginPlay();

    BroadcastManager = UBroadcastManager::Get(this);

    if (HasAuthority())
    {
        InitializeQuestList();
    }
}

void AQuestManagerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AQuestManagerActor, CurQuestIndex);
    DOREPLIFETIME(AQuestManagerActor, CurQuestTarget);
}

void AQuestManagerActor::StartQuest()
{
    if (!HasAuthority())
        return;

    if (!bInitialized)
    {
        InitializeQuestList();
        return;
    }

    ResetQuestProgress();
}

void AQuestManagerActor::NotifyContact(EBuildingType InType)
{
    if (HasAuthority())
        HandleContactInternal(InType);
    else
        ServerRPC_NotifyContact(InType);
}

void AQuestManagerActor::BroadcastQuestUpdate()
{
    HandleQuestUpdated();
}

void AQuestManagerActor::ServerRPC_NotifyContact_Implementation(EBuildingType InType)
{
    HandleContactInternal(InType);
}

void AQuestManagerActor::InitializeQuestList()
{
    if (bInitialized)
    {
        return;
    }

    QuestList = {
        EBuildingType::Sinpungnu,
        EBuildingType::Uhwagwan,
        EBuildingType::Jwaikmun,
        EBuildingType::Bongsudang,
        EBuildingType::Yeomingak,
        EBuildingType::Byeolju,
    };

    ResetQuestProgress();
    bInitialized = true;

    PRINTLOG(TEXT("[QUEST] Initialized quest list (%d entries)"), QuestList.Num());
}

void AQuestManagerActor::ResetQuestProgress()
{
    if (QuestList.Num() > 0)
    {
        CurQuestIndex = 0;
        CurQuestTarget = QuestList[CurQuestIndex];
    }
    else
    {
        CurQuestIndex = INDEX_NONE;
        CurQuestTarget = EBuildingType::None;
    }

    HandleQuestUpdated();
}

void AQuestManagerActor::AdvanceQuest()
{
    if (!HasAuthority())
    {
        return;
    }

    if (!QuestList.IsValidIndex(CurQuestIndex))
    {
        PRINTLOG(TEXT("[QUEST] AdvanceQuest called with invalid index"));
        CurQuestTarget = EBuildingType::None;
        HandleQuestUpdated();
        return;
    }

    ++CurQuestIndex;

    if (QuestList.IsValidIndex(CurQuestIndex))
    {
        CurQuestTarget = QuestList[CurQuestIndex];
        PRINTLOG(TEXT("[QUEST] Advanced to quest %d (%s)"),CurQuestIndex, *ENUM_TO_NAME(EBuildingType, CurQuestTarget));
    }
    else
    {
        CurQuestTarget = EBuildingType::None;
        PRINTLOG(TEXT("[QUEST] Quest chain completed"));
    }

    HandleQuestUpdated();
}

void AQuestManagerActor::HandleQuestUpdated()
{
    if (!BroadcastManager)
    {
        BroadcastManager = UBroadcastManager::Get(this);
    }

    if (BroadcastManager)
    {
        BroadcastManager->SendUpdateQuest(CurQuestTarget);
    }
}

void AQuestManagerActor::HandleContactInternal(EBuildingType InType)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!HasActiveQuest())
    {
        PRINTLOG(TEXT("[QUEST] Contact ignored. No active quest."));
        return;
    }

    if (InType != CurQuestTarget)
    {
        PRINTLOG(TEXT("[QUEST] Contact %s ignored. Current target is %s"),
            *ENUM_TO_NAME(EBuildingType, InType),
            *ENUM_TO_NAME(EBuildingType, CurQuestTarget));
        return;
    }

    PRINTLOG(TEXT("[QUEST] Contact confirmed for %s"),  *ENUM_TO_NAME(EBuildingType, CurQuestTarget));
    AdvanceQuest();
}

void AQuestManagerActor::OnRep_CurQuestIndex()
{
}

void AQuestManagerActor::OnRep_CurQuestTarget()
{
    HandleQuestUpdated();
}