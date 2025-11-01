// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UMegaPopup.h"
#include "EBuildingType.h"
#include "FBuildingData.h"
#include "FBuildingAssetData.h"
#include "UBuildingDetailData.h"
#include "UGameDataManager.h"
#include "UBroadcastManager.h"
#include "UGameSoundManager.h"
#include "Components/Button.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/AudioComponent.h"

#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void UMegaPopup::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UMegaPopup::OnCloseButtonClicked);
	}
}


void UMegaPopup::UpdateBuildingInfo(const EBuildingType InBuildingType)
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
		DetailTitleText->SetText(FText::FromString(BuildingData.detailtitle));
		DetailText->SetText(FText::FromString(BuildingData.detaildesc));
		SubTitle->SetText(FText::FromString(BuildingData.subtile));
	}

	// 건물 에셋 정보 데이터 가져오기
	FBuildingAssetData AssetData;
	if ( DataManager->GetBuildingAssetData(InBuildingType, AssetData) )
	{
		UBuildingDetailData* DetailAsset = AssetData.BuildingDetailDataAsset.LoadSynchronous();

		if ( DetailAsset )
		{
			TSoftObjectPtr<UTexture2D> LoadedTexture;
			if ( DetailAsset->LoadBackgroundImage(LoadedTexture) && LoadedTexture.Get() )
				BackgroundImage->SetBrushFromTexture(LoadedTexture.Get());

			TSoftObjectPtr<USoundCue> LoadedCue;
			if ( DetailAsset->LoadSoundCue(LoadedCue) && LoadedCue.Get() )
			{
				// UGameSoundManager를 통해 대화 음성 재생 (기존 음성 자동 중지)
				if (auto SoundManager = UGameSoundManager::Get(GetWorld()))
				{
					SoundManager->PlayConversationVoice(LoadedCue.Get());
				}
			}
		}
	}
}

void UMegaPopup::OnClose()
{
	// UGameSoundManager를 통해 대화 음성 중지
	if (auto SoundManager = UGameSoundManager::Get(GetWorld()))
	{
		SoundManager->StopConversationVoice();
	}
}

void UMegaPopup::NativeDestruct()
{
	// UGameSoundManager를 통해 대화 음성 중지
	if (auto SoundManager = UGameSoundManager::Get(GetWorld()))
	{
		SoundManager->StopConversationVoice();
	}

	Super::NativeDestruct();
}

void UMegaPopup::OnCloseButtonClicked()
{
	if (UBroadcastManager* BroadcastManager = UBroadcastManager::Get(GetWorld()))
	{
		BroadcastManager->SendMegaPopupClosed();
	}
}
