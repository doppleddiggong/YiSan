// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "AudioCapture.h"
#include "Components/ActorComponent.h"
#include "UStreamingRecordSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioChunkReady, const TArray<uint8>&, PcmData);

UCLASS(ClassGroup=(Voice), meta=(BlueprintSpawnableComponent))
class YISAN_API UStreamingRecordSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UStreamingRecordSystem();
 
	UPROPERTY(BlueprintAssignable, Category = "Voice")
	FOnAudioChunkReady OnAudioChunkReady;

public:
	void StartRecording();
	void StopRecording();

private:
	void HandleOnCapture(const float* InAudio, const int32 InNumFrames, const int32 InNumChannels, const int32 InSampleRate);

	TUniquePtr<Audio::FAudioCapture> AudioCapture;

	int32 LastSampleRate  = 16000;  // 기본값
	int32 LastNumChannels = 1;
	bool bIsRecording = false;
};