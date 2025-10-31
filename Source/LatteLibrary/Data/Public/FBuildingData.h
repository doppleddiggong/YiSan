// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file FHitStopData.h
 * @brief FHitStopData 구조체에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "Engine/DataTable.h"
#include "FBuildingData.generated.h"

USTRUCT(BlueprintType)
struct FBuildingData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBuildingType Type = EBuildingType::Yeomingak;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float x = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float y = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float z = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString detailtitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString detaildesc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString subtile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString explain;
};