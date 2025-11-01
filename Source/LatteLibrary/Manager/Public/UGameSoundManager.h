// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UGameSoundManager.h
 * @brief UGameSoundManager 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "EGameSoundType.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UGameSoundManager.generated.h"

UCLASS()
class LATTELIBRARY_API UGameSoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    DEFINE_SUBSYSTEM_GETTER_INLINE(UGameSoundManager);

    UGameSoundManager();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category="Sound")
    void PlaySound(EGameSoundType Type, FVector Location);

	UFUNCTION(BlueprintCallable, Category="Sound")
    void PlaySound2D(EGameSoundType Type);

	UFUNCTION(BlueprintCallable, Category="Sound")
    void StopSound2D(const EGameSoundType Type);

	// 대화 음성 재생 (기존 대화 음성을 중지하고 새로운 음성 재생)
	UFUNCTION(BlueprintCallable, Category="Sound")
	UAudioComponent* PlayConversationVoice(USoundBase* Sound);

	// 현재 재생 중인 대화 음성 중지
	UFUNCTION(BlueprintCallable, Category="Sound")
	void StopConversationVoice();

	// 대화 음성이 재생 중인지 확인
	UFUNCTION(BlueprintCallable, Category="Sound")
	bool IsConversationVoicePlaying() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USoundData> SoundAsset;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TMap<EGameSoundType, TObjectPtr<class USoundBase>> SoundData;

	UPROPERTY()
	TMap<EGameSoundType, UAudioComponent*> ActiveSounds;

	// 현재 재생 중인 대화 음성 (MegaPopup, DasanExplain, PlayerAsk 공유)
	UPROPERTY()
	TObjectPtr<UAudioComponent> ConversationVoice;
};
