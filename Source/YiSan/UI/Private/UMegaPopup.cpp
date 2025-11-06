// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UMegaPopup.cpp
 * @brief UMegaPopup의 동작을 구현합니다.
 */
#include "UMegaPopup.h"
#include "EBuildingType.h"
#include "FBuildingData.h"
#include "FBuildingAssetData.h"
#include "GameLogging.h"
#include "UBuildingDetailData.h"
#include "UGameDataManager.h"
#include "UBroadcastManager.h"
#include "UGameSoundManager.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

#include "Engine/Texture2D.h"
#include "Sound/SoundCue.h"

#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Styling/SlateBrush.h"

void UMegaPopup::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
		CloseButton->OnClicked.AddDynamic(this, &UMegaPopup::OnCloseButtonClicked);
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
	if (DataManager->GetBuildingAssetData(InBuildingType, AssetData))
	{
		UBuildingDetailData* DetailAsset = AssetData.BuildingDetailDataAsset.LoadSynchronous();

		if (DetailAsset)
		{
			BuildDetailAsset = DetailAsset;

			const bool bUseMedia = DetailAsset->UseMedia;
			const bool bPlayedMedia = bUseMedia && PlayMedia(*DetailAsset);
			if (!bUseMedia || !bPlayedMedia)
			{
				DisplayBackgroundImage(*DetailAsset);
			}

			TSoftObjectPtr<USoundCue> LoadedCue;
			if (DetailAsset->LoadSoundCue(LoadedCue) && LoadedCue.Get())
			{
				if (auto SoundManager = UGameSoundManager::Get(GetWorld()))
					SoundManager->PlayConversationVoice(LoadedCue.Get());
			}
		}
	}

	BuildingType = InBuildingType;
}

void UMegaPopup::OnClose()
{
	StopMedia();

	if (auto SoundManager = UGameSoundManager::Get(GetWorld()))
		SoundManager->StopConversationVoice();
}

void UMegaPopup::NativeDestruct()
{
	StopMedia();

	if (auto SoundManager = UGameSoundManager::Get(GetWorld()))
		SoundManager->StopConversationVoice();

	Super::NativeDestruct();
}

bool UMegaPopup::PlayMedia(const UBuildingDetailData& DetailAsset)
{
	if (!MediaPlayer)
	{
		PRINTLOG(TEXT("UMegaPopup::PlayBackgroundMedia - MediaPlayer is not assigned."));
		return false;
	}

	if (!BackgroundMedia)
	{
		PRINTLOG(TEXT("UMegaPopup::PlayBackgroundMedia - MediaImage is not assigned."));
		return false;
	}

	TSoftObjectPtr<UMediaSource> LoadedMediaSource;
	if (!DetailAsset.LoadBackgroundVideoSource(LoadedMediaSource) || !LoadedMediaSource.Get())
	{
		PRINTLOG(TEXT("UMegaPopup::PlayBackgroundMedia - Failed to load media source."));
		return false;
	}

	StopMedia();

	bPendingMedia = true;
	BuildDetailAsset = &DetailAsset;

	MediaPlayer->OnMediaOpened.AddDynamic(this, &UMegaPopup::OnMediaOpened);
	MediaPlayer->OnMediaOpenFailed.AddDynamic(this, &UMegaPopup::OnMediaOpenFailed);

	if (!MediaPlayer->OpenSource(LoadedMediaSource.Get()))
	{
		PRINTLOG(TEXT("UMegaPopup::PlayBackgroundMedia - Failed to open media source."));
		ClearMediaDelegates();
		bPendingMedia = false;
		return false;
	}

	BackgroundMedia->SetVisibility(ESlateVisibility::Collapsed);

	return true;
}

void UMegaPopup::StopMedia()
{
	bPendingMedia = false;
	ClearMediaDelegates();

	if (MediaPlayer)
		MediaPlayer->Close();

	if (BackgroundImage)
		BackgroundImage->SetVisibility(ESlateVisibility::Visible);

	if (BackgroundMedia)
		BackgroundMedia->SetVisibility(ESlateVisibility::Collapsed);

	BuildDetailAsset = nullptr;
}

void UMegaPopup::ClearMediaDelegates()
{
	if (MediaPlayer)
	{
		MediaPlayer->OnMediaOpened.RemoveAll(this);
		MediaPlayer->OnMediaOpenFailed.RemoveAll(this);
	}
}


void UMegaPopup::OnMediaOpened(FString OpenedUrl)
{
	if (!MediaPlayer || !bPendingMedia)
		return;

	ClearMediaDelegates();

	bPendingMedia = false;

	if (!MediaPlayer->Play())
	{
		PRINTLOG(TEXT("UMegaPopup::HandleMediaOpened - Failed to start playback after open."));

		if (BuildDetailAsset)
			DisplayBackgroundImage(*BuildDetailAsset);
		return;
	}

	if (BackgroundMedia)
		BackgroundMedia->SetVisibility(ESlateVisibility::Visible);

	if (BackgroundImage)
		BackgroundImage->SetVisibility(ESlateVisibility::Collapsed);
}

void UMegaPopup::OnMediaOpenFailed(FString FailedUrl)
{
	if (!bPendingMedia)
		return;

	ClearMediaDelegates();
	bPendingMedia = false;

	if (BuildDetailAsset)
		DisplayBackgroundImage(*BuildDetailAsset);
}

void UMegaPopup::DisplayBackgroundImage(const UBuildingDetailData& DetailAsset)
{
	StopMedia();

	if (!BackgroundImage)
		return;

	TSoftObjectPtr<UTexture2D> LoadedTexture;
	if (DetailAsset.LoadBackgroundImage(LoadedTexture) && LoadedTexture.Get())
		BackgroundImage->SetBrushFromTexture(LoadedTexture.Get());

	BackgroundImage->SetVisibility(ESlateVisibility::Visible);

	if (BackgroundMedia)
		BackgroundMedia->SetVisibility(ESlateVisibility::Collapsed);
}

void UMegaPopup::OnCloseButtonClicked()
{
	if (UBroadcastManager* BroadcastManager = UBroadcastManager::Get(GetWorld()))
		BroadcastManager->SendMegaPopupClosed();
}
