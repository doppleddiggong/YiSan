// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UGameSoundManager.cpp
 * @brief UGameSoundManager의 동작을 구현합니다.
 */
#include "UGameSoundManager.h"
#include "USoundData.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "FComponentHelper.h"

#define SOUND_DATA_PATH TEXT("/Game/CustomContents/MasterData/DA_Sound.DA_Sound")

UGameSoundManager::UGameSoundManager()
{
	// FComponentHelper를 사용해 생성자에서 안전하게 에셋을 로드합니다.
	if (auto LoadedAsset = FComponentHelper::LoadAsset<USoundData>(SOUND_DATA_PATH))
	{
		for (const auto& Pair : LoadedAsset->SoundData)
		{
			SoundAsset = LoadedAsset;
			SoundData = SoundAsset->SoundData;
			
			TSoftObjectPtr<USoundBase> SoundAssetPtr = Pair.Value;
			if (!SoundAssetPtr.IsNull()) {
				SoundData.Add(Pair.Key, SoundAssetPtr.LoadSynchronous());
			}
		}
	}
	else
	{
		// 실패 시, 에디터 실행 시 Output Log에 에러를 출력합니다.
        UE_LOG(LogTemp, Error, TEXT("UGameSoundManager failed to load USoundData at path: %s"), SOUND_DATA_PATH);
	}
}

void UGameSoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// FSoftObjectPath AssetPath(SOUND_DATA_PATH);
	// USoundData* LoadedAsset = Cast<USoundData>(StaticLoadObject(USoundData::StaticClass(), nullptr, *AssetPath.ToString()));
	//
	// if (LoadedAsset)
	// {
	// 	for (const auto& Pair : LoadedAsset->SoundData)
	// 	{
	// 		SoundAsset = LoadedAsset;
	// 		SoundData = SoundAsset->SoundData;
	// 		
	// 		TSoftObjectPtr<USoundBase> SoundAssetPtr = Pair.Value;
	// 		if (!SoundAssetPtr.IsNull())
	// 		{
	// 			SoundData.Add(Pair.Key, SoundAssetPtr.LoadSynchronous());
	// 		}
	// 	}
	// }
	// else
	if (!SoundAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load SoundDataAsset from path"));
	}
}

void UGameSoundManager::PlaySound(const EGameSoundType Type, const FVector Location)
{
	if (TObjectPtr<USoundBase>* FoundSound = SoundData.Find(Type))
	{
		if (*FoundSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), *FoundSound, Location);
		}
	}
}


void UGameSoundManager::PlaySound2D(const EGameSoundType Type)
{
	if (TObjectPtr<USoundBase>* FoundSound = SoundData.Find(Type))
	{
		if (*FoundSound)
		{
			// 이미 재생 중인 사운드가 있다면 멈추고 제거
			if (UAudioComponent* ExistingComp = ActiveSounds.FindRef(Type))
			{
				ExistingComp->Stop();
				ActiveSounds.Remove(Type);
			}

			// 새로운 사운드 재생 및 저장
			if (UAudioComponent* NewComp = UGameplayStatics::SpawnSound2D(GetWorld(), *FoundSound))
			{
				ActiveSounds.Add(Type, NewComp);
			}
		}
	}
}

void UGameSoundManager::StopSound2D(const EGameSoundType Type)
{
	if (UAudioComponent* Comp = ActiveSounds.FindRef(Type))
	{
		Comp->Stop();
		ActiveSounds.Remove(Type);
	}
}

UAudioComponent* UGameSoundManager::PlayConversationVoice(USoundBase* Sound)
{
	if (!Sound)
		return nullptr;

	// 기존 대화 음성이 재생 중이면 중지
	StopConversationVoice();

	// 새로운 대화 음성 재생
	ConversationVoice = UGameplayStatics::SpawnSound2D(GetWorld(), Sound);
	return ConversationVoice;
}

void UGameSoundManager::StopConversationVoice()
{
	if (ConversationVoice && ConversationVoice->IsPlaying())
	{
		ConversationVoice->Stop();
	}
	ConversationVoice = nullptr;
}

bool UGameSoundManager::IsConversationVoicePlaying() const
{
	return ConversationVoice && ConversationVoice->IsPlaying();
}
