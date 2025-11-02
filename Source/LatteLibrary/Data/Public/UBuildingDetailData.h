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
	/**
	 * @brief 썸네일 이미지를 동기적으로 로드합니다.
	 * @param OutTexture 로드된 텍스처 에셋
	 * @return 성공 시 true
	 */
	bool LoadThumbnailImage(TSoftObjectPtr<class UTexture2D>& OutTexture) const;

	/**
	 * @brief 배경 이미지를 동기적으로 로드합니다.
	 * @param OutTexture 로드된 텍스처 에셋
	 * @return 성공 시 true
	 */
	bool LoadBackgroundImage(TSoftObjectPtr<class UTexture2D>& OutTexture) const;


	bool LoadBackgroundVideoSource(TSoftObjectPtr<class UMediaSource>& OutMediaSource) const;
	/**
	 * @brief 사운드 큐를 동기적으로 로드합니다.
	 * @param OutSoundCue 로드된 사운드 큐 에셋
	 * @return 성공 시 true
	 */
	bool LoadSoundCue(TSoftObjectPtr<class USoundCue>& OutSoundCue) const;

	// UI 썸네일 이미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=UI)
	TSoftObjectPtr<class UTexture2D> ThumbnailImage;

	// UI 팝업(전체 배경) 이미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=UI)
	TSoftObjectPtr<class UTexture2D> BackgroundImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=UI)
	TSoftObjectPtr<class UMediaSource> BackgroundVideoSource;
	
	// 사운드큐 (SoundWave는 큐 안에서 관리)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Sound)
	TSoftObjectPtr<class USoundCue> SoundCueAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=UI)
	bool UseMedia;
};