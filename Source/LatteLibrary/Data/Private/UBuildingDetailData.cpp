// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#include "UBuildingDetailData.h"
#include "GameLogging.h"
#include "MediaSource.h"
#include "Sound/SoundCue.h"

/**
 * @file UBuildingDetailData.cpp
 * @brief UBuildingDetailData의 동작을 구현합니다.
 */

/**
 * @brief 건물을 대표하는 썸네일 텍스처를 로드합니다.
 * @param OutTexture 성공 시 로드된 썸네일 에셋으로 채워집니다.
 * @return 에셋을 성공적으로 로드하면 @c true, 그렇지 않으면 @c false를 반환합니다.
 */
bool UBuildingDetailData::LoadThumbnailImage(TSoftObjectPtr<UTexture2D>& OutTexture) const
{
	OutTexture = ThumbnailImage.LoadSynchronous();
	if (!OutTexture)
	{
		PRINTLOG(TEXT("Failed to LoadThumbnailImage"));
		return false;
	}
	return true;
}

/**
 * @brief 건물 상세 정보에 연관된 배경 이미지를 로드합니다.
 * @param OutTexture 성공 시 로드된 배경 텍스처로 채워집니다.
 * @return 배경 이미지가 존재하여 로드되었다면 @c true를 반환합니다.
 */
bool UBuildingDetailData::LoadBackgroundImage(TSoftObjectPtr<UTexture2D>& OutTexture) const
{
	OutTexture = BackgroundImage.LoadSynchronous();
	if (!OutTexture)
	{
		PRINTLOG(TEXT("Failed to LoadBackgroundImage"));
		return false;
	}
	return true;
}

/**
 * @brief 선택적인 배경 영상을 로드합니다.
 * @param OutMediaSource 사용 가능한 경우 로드된 미디어 소스로 채워집니다.
 * @return 미디어 소스가 올바르게 설정되고 로드되었다면 @c true를 반환합니다.
 */
bool UBuildingDetailData::LoadBackgroundVideoSource(TSoftObjectPtr<UMediaSource>& OutMediaSource) const
{
	if (BackgroundVideoSource.IsNull())
	{
		PRINTLOG(TEXT("BackgroundVideoSource is not set"));
		return false;
	}

	OutMediaSource = BackgroundVideoSource.LoadSynchronous();
	if (!OutMediaSource)
	{
		PRINTLOG(TEXT("Failed to LoadBackgroundVideoSource"));
		return false;
	}
	return true;
}

/**
 * @brief 상세 뷰에 사용되는 사운드 큐를 로드합니다.
 * @param OutSoundCue 사용 가능한 경우 로드된 큐 에셋으로 채워집니다.
 * @return 큐 에셋을 성공적으로 로드했다면 @c true를 반환합니다.
 */
bool UBuildingDetailData::LoadSoundCue(TSoftObjectPtr<USoundCue>& OutSoundCue) const
{
	OutSoundCue = SoundCueAsset.LoadSynchronous();
	if (!OutSoundCue)
	{
		PRINTLOG(TEXT("Failed to LoadSoundCue"));
		return false;
	}
	return true;
}