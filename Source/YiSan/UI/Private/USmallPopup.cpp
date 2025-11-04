// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


/**
 * @file USmallPopup.cpp
 * @brief USmallPopup의 동작을 구현합니다.
 */
#include "USmallPopup.h"
#include "EBuildingType.h"
#include "FBuildingData.h"
#include "FBuildingAssetData.h"
#include "UBuildingDetailData.h"
#include "UGameDataManager.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"

#include "Engine/Texture2D.h"
 
void USmallPopup::UpdateBuildingInfo(const EBuildingType InBuildingType)
{
	if (BuildingType == InBuildingType)
		return;
	
	// 데이터 매니저 가져오기
	auto DataManager = UGameDataManager::Get(GetWorld());
	if (!DataManager)
		return;

	// 건물 테이블 정보 데이터 가져오기
	FBuildingData BuildingData;
	if (DataManager->GetBuildingData(InBuildingType, BuildingData))
	{
		TitleText->SetText(FText::FromString(BuildingData.detailtitle));
		DescText->SetText(FText::FromString(BuildingData.description));
	}

	// 건물 에셋 정보 데이터 가져오기
	FBuildingAssetData AssetData;
	if ( DataManager->GetBuildingAssetData(InBuildingType, AssetData) )
	{
		UBuildingDetailData* DetailAsset = AssetData.BuildingDetailDataAsset.LoadSynchronous();

		if ( DetailAsset )
		{
			TSoftObjectPtr<UTexture2D> LoadedTexture;
			if ( DetailAsset->LoadThumbnailImage(LoadedTexture) && LoadedTexture.Get() )
				ThumbnailImage->SetBrushFromTexture(LoadedTexture.Get());
		}
	}
}

