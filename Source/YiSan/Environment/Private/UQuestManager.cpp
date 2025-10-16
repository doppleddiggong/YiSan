// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UQuestManager.h"
#include "GameLogging.h"
#include "UBroadcastManager.h"

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UQuestManager::InitSystem()
{
    QuestList = {
        EBuildingType::Sinpungnu,     
        EBuildingType::Uhwagwan,      
        EBuildingType::Jwaikmun,      
        EBuildingType::Bongsudang,    
        EBuildingType::Yeomingak,     
        EBuildingType::Byeolju,
    };

    CurQuestIndex = 0;
    CurTarget = QuestList[CurQuestIndex];
    
    BroadcastManager = UBroadcastManager::Get(GetWorld());
    BroadcastManager->OnContactBuilding.AddDynamic(this, &UQuestManager::OnContactBuilding);
}


void UQuestManager::Deinitialize()
{
    Super::Deinitialize();
}

void UQuestManager::OnContactBuilding(EBuildingType InType)
{
    if (CurTarget == EBuildingType::None)
        return;

    if (InType == CurTarget)
    {
        const UEnum* EnumPtr = StaticEnum<EBuildingType>();
        const FString TargetName = EnumPtr->GetNameStringByValue((int64)CurTarget);

        PRINT_STRING(TEXT("Quest Completed: %s"), *TargetName);

        CurQuestIndex++;
        if (QuestList.IsValidIndex(CurQuestIndex))
        {
            CurTarget = QuestList[CurQuestIndex];
            BroadcastManager->SendUpdateQuest(CurTarget);
        }
    }
}