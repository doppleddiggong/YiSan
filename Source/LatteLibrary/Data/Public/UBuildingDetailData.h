// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UBuildingDetailData.generated.h"

UCLASS(BlueprintType)
class LATTELIBRARY_API UBuildingDetailData : public UDataAsset
{
	GENERATED_BODY()

public:
	bool LoadThumbnailImage(TSoftObjectPtr<UTexture2D>& OutTexture) const;
	bool LoadBackgroundImage(TSoftObjectPtr<UTexture2D>& OutTexture) const;
	bool LoadBackgroundVideoSource(TSoftObjectPtr<class UMediaSource>& OutMediaSource) const;
	bool LoadSoundCue(TSoftObjectPtr<class USoundCue>& OutSoundCue) const;

	UFUNCTION(BlueprintCallable, Category = "Building Detail")
	bool HasBackgroundVideo() const { return !BackgroundVideoSource.IsNull(); }

	// UI 썸네일 이미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=UI)
	TSoftObjectPtr<class UTexture2D> ThumbnailImage;

	// UI 팝업(전체 배경) 이미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=UI)
	TSoftObjectPtr<class UTexture2D> BackgroundImage;

	// 배경 비디오 사용 여부 체크박스 (먼저 선언!)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=UI)
	bool bUseBackgroundVideo = false;

	// UI 팝업 배경 비디오 - EditCondition 제거하거나 수정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=UI)
	TSoftObjectPtr<class UMediaSource> BackgroundVideoSource;

	// 사운드큐
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Sound)
	TSoftObjectPtr<class USoundCue> SoundCueAsset;
};