// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UAudioCaptureComponent.generated.h"

// class UVoiceSystem;
// class UAudioComponent;
// class USoundWaveProcedural;
// class UGoogleSpeechService;
// class IVoiceCapture;


/**
 * @brief Handles push-to-talk capture, STT dispatch, and TTS playback for a character.
 *
 * This component is designed to sit next to `UVoiceSystem`, reusing its correlation ids and
 * observability logging while delegating network calls to `UGoogleSpeechService`.
 * @ingroup Character
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Dopple), meta=(BlueprintSpawnableComponent))
class YISAN_API UVoiceCaptureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoiceCaptureComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/** @brief Bind the character voice system explicitly (optional, will auto-discover on BeginPlay). */
	UFUNCTION(BlueprintCallable, Category="Voice")
	void SetVoiceSystem(class UVoiceSystem* InVoiceSystem);

	/** @brief Called when push-to-talk key is pressed. */
	UFUNCTION(BlueprintCallable, Category="Voice|Capture")
	void HandlePushToTalkPressed(const FString& ContextTag);

	/** @brief Called when push-to-talk key is released. */
	UFUNCTION(BlueprintCallable, Category="Voice|Capture")
	void HandlePushToTalkReleased();

	/**
	 * @brief Synthesize arbitrary user text via Google Text-to-Speech.
	 * @param Text Arbitrary text to synthesize.
	 */
	UFUNCTION(BlueprintCallable, Category="Voice|Synthesis")
	void RequestSynthesis(const FString& Text);

public:
	/** @brief Speech synthesis callback. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceSynthesisReady, const FString&, CorrelationId, USoundWaveProcedural*, SynthesizedAudio);
	/** @brief Fired when synthesized audio is ready (LINEAR16 stream wrapped into a procedural sound wave). */
	UPROPERTY(BlueprintAssignable, Category="Voice|Delegates")
	FOnVoiceSynthesisReady OnVoiceSynthesisReady;

private:
	void EnsureServices();
	bool EnsureVoiceCapture();
	void StartAudioCapture();
	void StopAudioCapture();
	void DrainVoiceCapture();

	void DispatchSpeechToText(const FString& CorrelationId, const TArray<uint8>& AudioPayload);
	void HandleSpeechToTextCompleted(bool bSuccess, const FString& CorrelationId, const FString& Transcript, float Confidence);

	void DispatchTextToSpeech(const FString& CorrelationId, const FString& Text);
	void HandleTextToSpeechCompleted(bool bSuccess, const FString& CorrelationId, const FString& Text, const TArray<uint8>& AudioData, int32 SampleRate);

	class USoundWaveProcedural* CreateSoundWaveFromPcm(const TArray<uint8>& AudioData, int32 SampleRate, int32 NumChannels) const;

private:
	/** @brief Correlation-aware session manager component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Voice", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UVoiceSystem> VoiceSystem;

	/** @brief Cached Google speech service reference. */
	UPROPERTY(Transient)
	TObjectPtr<class UGoogleSpeechService> SpeechService;

	/** @brief Voice capture interface provided by VoiceModule. */
	TSharedPtr<class IVoiceCapture> VoiceCapture;

	/** @brief PCM buffer captured during the current session (16-bit little endian). */
	TArray<uint8> CaptureBuffer;

	/** @brief Scratch buffer reused while draining the capture stream. */
	TArray<uint8> ScratchBuffer;

	/** @brief Sample rate (Hz) for STT payload. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Voice|Config", meta=(ClampMin="8000", ClampMax="48000", AllowPrivateAccess="true"))
	int32 CaptureSampleRate = 16000;

	/** @brief Number of channels captured from the microphone. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Voice|Config", meta=(ClampMin="1", ClampMax="2", AllowPrivateAccess="true"))
	int32 CaptureNumChannels = 1;

	/** @brief True while we are currently capturing microphone audio. */
	bool bIsCapturing = false;

	/** @brief Most recent correlation id used during capture (kept after StopCapture for async replies). */
	FString LastCorrelationId;
};
