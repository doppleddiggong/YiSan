// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "Engine/DataTable.h"
#include "FBuildingAssetData.generated.h"

USTRUCT(BlueprintType)
struct LATTELIBRARY_API FBuildingAssetData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Asset)
	EBuildingType BuildingType = EBuildingType::Max;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Asset)
	TSoftObjectPtr<class UBuildingDetailData> BuildingDetailDataAsset;
};
