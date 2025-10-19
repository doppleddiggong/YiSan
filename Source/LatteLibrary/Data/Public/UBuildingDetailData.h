// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UCharacterData.h
 * @brief UCharacterData 데이터 자산에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UBuildingDetailData.generated.h"

UCLASS(BlueprintType)
class LATTELIBRARY_API UBuildingDetailData : public UDataAsset
{
	GENERATED_BODY()
public:
	// UI 썸네일 이미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=UI)
	TSoftObjectPtr<class UTexture2D> ThumbnailImage;

	// UI 팝업(전체 배경) 이미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=UI)
	TSoftObjectPtr<class UTexture2D> BackgroundImage;

	// 사운드큐 (SoundWave는 큐 안에서 관리)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Sound)
	TSoftObjectPtr<class USoundCue> SoundCueAsset;
};
