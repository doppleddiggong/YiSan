// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "ABuilding.h"

#include "APlayerActor.h"
#include "UBroadcastManager.h"
#include "Components/BoxComponent.h"
#include "YiSan/YiSan.h"

ABuilding::ABuilding()
{
	PrimaryActorTick.bCanEverTick = true;

	Tags.Add(GameTags::Building);

	boxcomp = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	boxcomp->SetupAttachment(GetRootComponent());
}

void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsNextTargetBuilding)
	{
		const FString BuildingTypeName = StaticEnum<EBuildingType>()->GetNameStringByValue(static_cast<int64>(BuildingType));
		const FString DebugText = FString::Printf(TEXT("%s::빛 기둥"), *BuildingTypeName);
		DrawDebugString(GetWorld(), GetActorLocation(), DebugText, nullptr, FColor::White, 0.0f, true);
	}
}

void ABuilding::BeginPlay()
{
	Super::BeginPlay();

	if ( auto EventManager = UBroadcastManager::Get(GetWorld()))
	{
		EventManager->OnUpdateQuest.AddDynamic(this, &ABuilding::OnUpdateQuest);
	}
}

void ABuilding::OnUpdateQuest(EBuildingType InBuildingType)
{
	bIsNextTargetBuilding = BuildingType == InBuildingType;
}