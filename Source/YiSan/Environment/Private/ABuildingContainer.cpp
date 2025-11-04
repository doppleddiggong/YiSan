// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "ABuildingContainer.h"

#include "ABuilding.h"
#include "Macro.h"
#include "EngineUtils.h"
#include "GameLogging.h"

/**
 * @file ABuildingContainer.cpp
 * @brief ABuildingContainer의 동작을 구현합니다.
 */

/** @brief 패시브 컨테이너 액터의 Tick을 비활성화합니다. */
ABuildingContainer::ABuildingContainer()
{
	PrimaryActorTick.bCanEverTick = false;
}

/** @brief 액터 초기화 시 건물 참조를 수집합니다. */
void ABuildingContainer::BeginPlay()
{
	Super::BeginPlay();

	RefreshBuildingMap();
}

/** @brief 현재 월드의 건물 액터 캐시 맵을 갱신합니다. */
void ABuildingContainer::CollectBuildings()
{
	RefreshBuildingMap();
}

/**
 * @brief 지정된 유형에 맞는 건물 액터를 검색합니다.
 * @param InBuildingType 건물 맵을 조회할 때 사용하는 키입니다.
 * @return 일치하는 건물 인스턴스를 반환하고 없으면 @c nullptr을 반환합니다.
 */
ABuilding* ABuildingContainer::GetBuilding(const EBuildingType InBuildingType) const
{
	if (const TObjectPtr<ABuilding>* const Found = BuildingMap.Find(InBuildingType))
		return Found->Get();
	return nullptr;
}

/** @brief 레벨의 액터를 순회하여 내부 건물 맵을 재구성합니다. */
void ABuildingContainer::RefreshBuildingMap()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	BuildingMap.Reset();

	for (TActorIterator<ABuilding> It(World); It; ++It)
	{
		ABuilding* const Building = *It;
		if (!IsValid(Building))
			continue;

		const EBuildingType BuildingType = Building->BuildingType;
		if (BuildingType == EBuildingType::None)
			continue;

		if (const TObjectPtr<ABuilding>* const Existing = BuildingMap.Find(BuildingType))
			PRINTLOG( TEXT("Duplicate building type %s found"), *FString(ENUM_TO_NAME(EBuildingType, BuildingType) ));

		BuildingMap.Add(BuildingType, Building);
	}
}