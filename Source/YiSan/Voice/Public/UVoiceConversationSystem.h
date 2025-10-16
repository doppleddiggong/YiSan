// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UVoiceConversationSystem.h
 * @brief 음성 대화 통합 시스템 - STT, GPT, TTS를 하나의 흐름으로 관리
 */
#pragma once

#include "CoreMinimal.h"
#include "NetworkData.h"
#include "AudioCapture.h"
#include "Components/ActorComponent.h"
#include "UVoiceConversationSystem.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Voice), meta=(BlueprintSpawnableComponent))
class YISAN_API UVoiceConversationSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoiceConversationSystem();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void InitSystem(class APlayerActor* InOwner);

	/** 음성 녹음 시작 */
	UFUNCTION(BlueprintCallable, Category = "Voice|Conversation")
	void StartRecording();

	/** 음성 녹음 중지 및 ASK 처리 시작 */
	UFUNCTION(BlueprintCallable, Category = "Voice|Conversation")
	void StopRecording();

private:
	void HandleOnCapture(const float* InAudio, int32 InNumFrames, int32 InNumChannels, int32 InSampleRate);

	UFUNCTION()
	void OnResponseAsk(FResponseAsk& Response, bool bSuccess);

private:
	UPROPERTY()
	TObjectPtr<class APlayerActor> Owner;
	UPROPERTY()
	TObjectPtr<class UBroadcastManager> BroadcastManager;


	// --- 녹음 관련 변수 ---
	TUniquePtr<Audio::FAudioCapture> AudioCapture;
	
	TArray<uint8> WAVData;
	TArray<uint8> PCMData;

	int32 LastSampleRate  = 16000;
	int32 LastNumChannels = 1;
	FString LastRecordedFilePath;
	
	// --- 상태 변수 ---
	bool bIsRecording = false;
	bool bIsProcessing = false;
};