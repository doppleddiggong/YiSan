// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "GameFramework/Actor.h"
#include "ABuildingContainer.generated.h"

UCLASS()
class YISAN_API ABuildingContainer : public AActor
{
	GENERATED_BODY()

public:
	ABuildingContainer();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(CallInEditor, Category="BuildingContainer", meta=(DevelopmentOnly, DisplayName="Collect Buildings"))
	void CollectBuildings();

	UFUNCTION(BlueprintCallable, Category="BuildingContainer")
	class ABuilding* GetBuilding(EBuildingType InBuildingType) const;

protected:
	void RefreshBuildingMap();

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="BuildingContainer")
	TMap<EBuildingType, TObjectPtr<class ABuilding>> BuildingMap;
};
