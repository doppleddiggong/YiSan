// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UGameDataManager.cpp
 * @brief UGameDataManager의 동작을 구현합니다.
 */
#include "UGameDataManager.h"
#include "GameLogging.h"
#include "FCharacterAssetData.h"
#include "FBuildingAssetData.h"
#include "FComponentHelper.h"

#define HITSTOP_PATH    TEXT("/Game/CustomContents/MasterData/DT_HitStop.DT_HitStop")
#define KNOCKBACK_PATH  TEXT("/Game/CustomContents/MasterData/DT_Knockback.DT_Knockback")
#define BUILDINGDATA_PATH  TEXT("/Game/CustomContents/MasterData/DT_Building.DT_Building")
#define BUILDINGASSET_PATH  TEXT("/Game/CustomContents/MasterData/DT_BuildingAsset.DT_BuildingAsset")
#define CHARACTERINFO_PATH  TEXT("/Game/CustomContents/MasterData/DT_CharacterInfo.DT_CharacterInfo")
#define CHARACTERASSET_PATH  TEXT("/Game/CustomContents/MasterData/DT_CharacterAsset.DT_CharacterAsset")

UGameDataManager::UGameDataManager()
{
    // HitStopTable = FComponentHelper::LoadAsset<UDataTable>(HITSTOP_PATH);
    // KnockbackTable  = FComponentHelper::LoadAsset<UDataTable>(KNOCKBACK_PATH);
    BuildingDataTable = FComponentHelper::LoadAsset<UDataTable>(BUILDINGDATA_PATH);
    BuildingAssetTable  = FComponentHelper::LoadAsset<UDataTable>(BUILDINGASSET_PATH);
    // CharacterInfoTable  = FComponentHelper::LoadAsset<UDataTable>(CHARACTERINFO_PATH);
    // CharacterAssetTable = FComponentHelper::LoadAsset<UDataTable>(CHARACTERASSET_PATH);
}

void UGameDataManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ReloadMasterData();
}

void UGameDataManager::Deinitialize()
{
    Clear_BuildingDataTable();
    Clear_BuildingAssetData();

    Clear_HitStopTable();
    Clear_KnockbackTable();
    Clear_CharacterInfoData();
    Clear_CharacterAssetData();
    
    Super::Deinitialize();
}

void UGameDataManager::ReloadMasterData()
{
    LoadData_BuildingDataTable();
    LoadData_BuildingAssetData();

    // LoadData_HitStopTable();
    // LoadData_KnockbackTable();
    // LoadData_CharacterInfoData();
    // LoadData_CharacterAssetData();
}

#pragma region BUILDING_DATA
void UGameDataManager::Clear_BuildingDataTable()
{
    BuildingDataCache.Reset();
    bLoadBuildingData  = false;   
}

void UGameDataManager::LoadData_BuildingDataTable()
{
    BuildingDataCache.Reset();
    bLoadBuildingData  = false;

    UDataTable* TableObj = BuildingDataTable.LoadSynchronous();
    if (!TableObj)
    {
        PRINTLOG(TEXT("Load failed: %s"), *BuildingDataTable.ToString());
        return;
    }

    static const FString ContextString(TEXT("BuildingDataTable"));
    for (const FName& RowName : TableObj->GetRowNames() )
    {
        if (const FBuildingData* Row = TableObj->FindRow<FBuildingData>(RowName, ContextString, true))
        {
            BuildingDataCache.Add(Row->Type, *Row);
        }
    }

    bLoadBuildingData  = true;
}

bool UGameDataManager::GetBuildingData(EBuildingType Type, FBuildingData& Out) const
{
    if (!bLoadBuildingData )
        return false;

    if (const FBuildingData* Found = BuildingDataCache.Find(Type))
    {
        Out = *Found;
        return true;
    }

    PRINTLOG(TEXT("DataGetFail : %s"), *UEnum::GetValueAsString(Type) );
    return false;
}

FString UGameDataManager::GetBuildingDataName(EBuildingType Type) const
{
    if (!bLoadBuildingData)
        return TEXT("");

    if (const FBuildingData* Found = BuildingDataCache.Find(Type))
        return Found->name;

    return TEXT("");
}
#pragma endregion BUILDING_DATA


#pragma region BUILDING_ASSET_DATA
void UGameDataManager::Clear_BuildingAssetData()
{
    BuildingAssetCache.Reset();
    bLoadBuildingAsset = false;
}

void UGameDataManager::LoadData_BuildingAssetData()
{
    BuildingAssetCache.Reset();
    bLoadBuildingAsset = false;

    UDataTable* TableObj = BuildingAssetTable.LoadSynchronous();
    if (!TableObj)
    {
        PRINTLOG(TEXT("Load failed: %s"), *BuildingAssetTable.ToString());
        return;
    }

    static const FString ContextString(TEXT("BuildingAssetTable"));
    for (const FName& RowName : TableObj->GetRowNames())
    {
        if (const FBuildingAssetData* Row = TableObj->FindRow<FBuildingAssetData>(RowName, ContextString, true))
        {
            BuildingAssetCache.Add(Row->BuildingType, *Row);
        }
    }

    bLoadBuildingAsset = true;
}

bool UGameDataManager::GetBuildingAssetData(EBuildingType Type, FBuildingAssetData& Out) const
{
    if (!bLoadBuildingAsset)
        return false;

    if (const FBuildingAssetData* Found = BuildingAssetCache.Find(Type))
    {
        Out = *Found;
        return true;
    }

    PRINTLOG(TEXT("DataGetFail : %s"), *UEnum::GetValueAsString(Type));
    return false;
}
#pragma endregion BUILDING_ASSET_DATA




#pragma region HIT_STOP
void UGameDataManager::Clear_HitStopTable()
{
    HitStopCache.Reset();
    bLoadHitStop = false;   
}

void UGameDataManager::LoadData_HitStopTable()
{
    HitStopCache.Reset();
    bLoadHitStop = false;

    UDataTable* TableObj = HitStopTable.LoadSynchronous();
    if (!TableObj)
    {
        PRINTLOG(TEXT("Load failed: %s"), *HitStopTable.ToString());
        return;
    }

    static const FString ContextString(TEXT("HitStopTable"));
    for (const FName& RowName : TableObj->GetRowNames() )
    {
        if (const FHitStopData* Row = TableObj->FindRow<FHitStopData>(RowName, ContextString, true))
        {
            HitStopCache.Add(Row->Type, *Row);
        }
    }

    bLoadHitStop = true;
}

bool UGameDataManager::GetHitStopData(EDamageType Type, FHitStopData& Out) const
{
    if (!bLoadHitStop)
        return false;

    if (const FHitStopData* Found = HitStopCache.Find(Type))
    {
        Out = *Found;
        return true;
    }

    PRINTLOG(TEXT("DataGetFail : %s"), *UEnum::GetValueAsString(Type) );
    return false;
}

float UGameDataManager::GetHitStopDelayTime(EDamageType Type) const
{
    if (!bLoadHitStop)
        return 0.0f;

    if (const FHitStopData* Found = HitStopCache.Find(Type))
        return Found->Duration;
    
    return 0.0f;
}

#pragma endregion HIT_STOP

#pragma region KNOCKBACK
void UGameDataManager::Clear_KnockbackTable()
{
    KnockbackCache.Reset();
    bLoadKnockback = false;   
}

void UGameDataManager::LoadData_KnockbackTable()
{
    KnockbackCache.Reset();
    bLoadKnockback = false;

    UDataTable* TableObj = KnockbackTable.LoadSynchronous();
    if (!TableObj)
    {
        PRINTLOG(TEXT("Load failed: %s"), *KnockbackTable.ToString());
        return;
    }

    static const FString ContextString(TEXT("KnockbackTable"));
    for (const FName& RowName : TableObj->GetRowNames() )
    {
        if (const FKnockbackData* Row = TableObj->FindRow<FKnockbackData>(RowName, ContextString, true))
        {
            KnockbackCache.Add(Row->Type, *Row);
        }
    }

    bLoadKnockback = true;
}

bool UGameDataManager::GetKnockbackData(EDamageType Type, FKnockbackData& Out) const
{
    if (!bLoadKnockback)
        return false;

    if (const FKnockbackData* Found = KnockbackCache.Find(Type))
    {
        Out = *Found;
        return true;
    }

    PRINTLOG(TEXT("DataGetFail : %s"), *UEnum::GetValueAsString(Type) );
    return false;
}
#pragma endregion KNOCKBACK

#pragma region CHARACTER_INFO_DATA
void UGameDataManager::Clear_CharacterInfoData()
{
    CharacterInfoCache.Reset();
    bLoadCharacterInfo = false;   
}

void UGameDataManager::LoadData_CharacterInfoData()
{
    CharacterInfoCache.Reset();
    bLoadCharacterInfo = false;

    UDataTable* TableObj = CharacterInfoTable.LoadSynchronous();
    if (!TableObj)
    {
        PRINTLOG(TEXT("Load failed: %s"), *CharacterInfoTable.ToString());
        return;
    }

    static const FString ContextString(TEXT("CharacterInfoTable"));
    for (const FName& RowName : TableObj->GetRowNames() )
    {
        if (const FCharacterInfoData* Row = TableObj->FindRow<FCharacterInfoData>(RowName, ContextString, true))
        {
            CharacterInfoCache.Add(Row->Type, *Row);
        }
    }

    bLoadCharacterInfo = true;
}

bool UGameDataManager::GetCharacterInfoData(ECharacterType Type, FCharacterInfoData& Out) const
{
    if (!bLoadCharacterInfo)
        return false;

    if (const FCharacterInfoData* Found = CharacterInfoCache.Find(Type))
    {
        Out = *Found;
        return true;
    }

    PRINTLOG(TEXT("DataGetFail : %s"), *UEnum::GetValueAsString(Type) );
    return false;
}
#pragma endregion CHARACTER_INFO_DATA

#pragma region CHARACTER_ASSET_DATA
void UGameDataManager::Clear_CharacterAssetData()
{
    CharacterAssetCache.Reset();
    bLoadCharacterAsset = false;   
}

void UGameDataManager::LoadData_CharacterAssetData()
{
    CharacterAssetCache.Reset();
    bLoadCharacterAsset = false;

    UDataTable* TableObj = CharacterAssetTable.LoadSynchronous();
    if (!TableObj)
    {
        PRINTLOG(TEXT("Load failed: %s"), *CharacterAssetTable.ToString());
        return;
    }

    static const FString ContextString(TEXT("CharacterAssetTable"));
    for (const FName& RowName : TableObj->GetRowNames() )
    {
        if (const FCharacterAssetData* Row = TableObj->FindRow<FCharacterAssetData>(RowName, ContextString, true))
        {
            CharacterAssetCache.Add(Row->CharacterType, *Row);
        }
    }

    bLoadCharacterAsset = true;
}

bool UGameDataManager::GetCharacterAssetData(ECharacterType Type, FCharacterAssetData& Out) const
{
    if (!bLoadCharacterAsset)
        return false;

    if (const FCharacterAssetData* Found = CharacterAssetCache.Find(Type))
    {
        Out = *Found;
        return true;
    }

    PRINTLOG(TEXT("DataGetFail : %s"), *UEnum::GetValueAsString(Type) );
    return false;
}
#pragma endregion CHARACTER_ASSET_DATA
