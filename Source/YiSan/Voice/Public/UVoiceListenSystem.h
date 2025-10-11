// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UVoiceListenSystem.generated.h"

UCLASS(ClassGroup=(Voice), meta=(BlueprintSpawnableComponent))
class YISAN_API UVoiceListenSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoiceListenSystem();
	
	void InitSystem();
	void HandleTTSOutput(const TArray<uint8>& AudioData, UObject* WorldContextObject);

protected:
	UFUNCTION()
	void HandleAudioStart();
	UFUNCTION()
	void HandleAudioChunk(const TArray<uint8>& AudioData);
	UFUNCTION()
	void HandleAudioEnd();



private:
	UPROPERTY()
	TObjectPtr<class UAudioComponent> AudioComponent;

	UPROPERTY()
	TObjectPtr<class USoundWaveProcedural> ProceduralSoundWave;
};