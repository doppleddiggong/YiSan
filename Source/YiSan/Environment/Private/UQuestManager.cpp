// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UQuestManager.h"
#include "GameLogging.h"
#include "UBroadcastManager.h"

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

FString UQuestManager::GetTargetBuildingName()
{
    if ( CurTarget == EBuildingType::None)
        return TEXT("EMPTY");
    
    return ENUM_TO_NAME( EBuildingType, CurTarget );
}

void UQuestManager::OnContactBuilding(EBuildingType InType)
{
    if (CurTarget == EBuildingType::None)
        return;

    if (InType == CurTarget)
    {
        PRINTLOG(TEXT("Quest Completed: %s"), *ENUM_TO_NAME(EBuildingType, CurTarget));

        CurQuestIndex++;
        if (QuestList.IsValidIndex(CurQuestIndex))
        {
            CurTarget = QuestList[CurQuestIndex];
            BroadcastManager->SendUpdateQuest(CurTarget);
        }
    }
}