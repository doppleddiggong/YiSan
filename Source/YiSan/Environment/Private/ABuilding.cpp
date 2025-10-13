// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "ABuilding.h"
#include "YiSan/YiSan.h"

ABuilding::ABuilding()
{
	PrimaryActorTick.bCanEverTick = false;

	Tags.Add(GameTags::Building);
}

void ABuilding::BeginPlay()
{
	Super::BeginPlay();
}