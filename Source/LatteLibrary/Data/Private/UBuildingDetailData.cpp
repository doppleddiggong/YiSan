// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#include "UBuildingDetailData.h"
#include "GameLogging.h"
#include "MediaSource.h"
#include "Sound/SoundCue.h"

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