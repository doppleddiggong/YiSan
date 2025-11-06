// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

/**
 * @file FBuildingAssetData.h
 * @brief UBuildingDetailData 클래스를 선언합니다.
 */

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "Engine/DataTable.h"
#include "FBuildingAssetData.generated.h"

USTRUCT(BlueprintType)
struct LATTELIBRARY_API FBuildingAssetData : public FTableRowBase
{
        GENERATED_BODY()

        /** @brief 지역화 세부 데이터를 조회할 때 사용하는 건물 유형 식별자입니다. */
        
        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Asset)
        EBuildingType BuildingType = EBuildingType::Max;

        /** @brief 썸네일, 오디오, 설명을 포함한 상세 에셋에 대한 소프트 참조입니다. */
        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Asset)
        TSoftObjectPtr<class UBuildingDetailData> BuildingDetailDataAsset;
};
