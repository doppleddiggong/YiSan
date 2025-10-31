// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UBuildingDetailData.h"
#include "GameLogging.h"
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