// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "GameFramework/Actor.h"
#include "ABuilding.generated.h"

UCLASS()
class YISAN_API ABuilding : public AActor
{
	GENERATED_BODY()

public:
	ABuilding();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBuildingType BuildingType = EBuildingType::Yeomingak;
};