// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "ABuildingContainer.h"

#include "ABuilding.h"
#include "Macro.h"
#include "EngineUtils.h"
#include "GameLogging.h"
#include "UObject/Package.h"

#if WITH_EDITOR
#include "Editor.h"
#include "ScopedTransaction.h"
#endif

ABuildingContainer::ABuildingContainer()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABuildingContainer::BeginPlay()
{
	Super::BeginPlay();

	RefreshBuildingMap();
}

void ABuildingContainer::CollectBuildings()
{
#if WITH_EDITOR
	const FScopedTransaction Transaction(
		NSLOCTEXT("BuildingContainer", "CollectBuildings", "Collect Buildings From Level")
	);
	Modify();
#endif

	RefreshBuildingMap();

#if WITH_EDITOR
	if (UPackage* Package = GetOutermost())
	{
		Package->SetDirtyFlag(true);
	}
#endif
}

void ABuildingContainer::FocusBuilding(const EBuildingType InBuildingType)
{
#if WITH_EDITOR
	if (!GEditor)
	{
		return;
	}

	ABuilding* Building = GetBuilding(InBuildingType);
	if (!IsValid(Building))
	{
		RefreshBuildingMap();
		Building = GetBuilding(InBuildingType);
	}

	if (!IsValid(Building))
	{
		return;
	}

	GEditor->SelectNone(false, true, false);
	GEditor->SelectActor(Building, true, true, true);

	TArray<AActor*> ActorsToFocus;
	ActorsToFocus.Add(Building);
	GEditor->MoveViewportCamerasToActor(ActorsToFocus, false);
#endif
}

ABuilding* ABuildingContainer::GetBuilding(const EBuildingType InBuildingType) const
{
	if (const TObjectPtr<ABuilding>* const Found = BuildingMap.Find(InBuildingType))
	{
		return Found->Get();
	}

	return nullptr;
}

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