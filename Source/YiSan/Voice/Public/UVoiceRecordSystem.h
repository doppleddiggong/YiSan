// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "AudioCapture.h"
#include "Components/ActorComponent.h"
#include "UVoiceRecordSystem.generated.h"

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecordingStopped, const FString&, FilePath);

UCLASS(ClassGroup=(Voice), meta=(BlueprintSpawnableComponent))
class YISAN_API UVoiceRecordSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoiceRecordSystem();

public:
	void RecordStart();
	FString RecordStop();

private:
	void HandleOnCapture(const float* InAudio, int32 InNumFrames, int32 InNumChannels, int32 InSampleRate);

	TUniquePtr<Audio::FAudioCapture> AudioCapture;
	TArray<uint8> WAVData;
	TArray<uint8> PCMData;

	int32 LastSampleRate  = 16000;  // 기본값
	int32 LastNumChannels = 1;
	bool bIsRecording = false;

	FString LastRecordedFilePath;
};