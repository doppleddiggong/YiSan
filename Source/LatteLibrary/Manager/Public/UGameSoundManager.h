// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

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
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USoundData> SoundAsset;
	
	UPROPERTY(EditAnywhere, Category = "Sound")
	TMap<EGameSoundType, TObjectPtr<class USoundBase>> SoundData;

	UPROPERTY()
	TMap<EGameSoundType, UAudioComponent*> ActiveSounds;
};
