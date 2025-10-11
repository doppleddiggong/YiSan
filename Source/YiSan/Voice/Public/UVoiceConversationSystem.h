// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UVoiceConversationSystem.h
 * @brief 음성 대화 통합 시스템 - STT, GPT, TTS를 하나의 흐름으로 관리
 */
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NetworkData.h"
#include "Macro.h"
#include "UVoiceConversationSystem.generated.h"

// --- 대화 이벤트 델리게이트 ---

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConversationRecordingStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConversationTranscriptionReceived, const FString&, TranscribedText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConversationGPTResponseReceived, const FString&, GPTResponse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConversationTTSStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConversationCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConversationError, const FString&, ErrorMessage);

/**
 * @brief 음성 대화 통합 시스템
 *
 * HTTP 방식과 WebSocket 방식 모두 지원:
 *
 * [HTTP 방식]
 * 1. StartRecording() → 음성 녹음 시작
 * 2. StopRecording() → 녹음 중지 및 WAV 파일 저장
 * 3. 자동으로 STT → GPT → TTS 처리
 * 4. TTS 오디오 재생
 *
 * [WebSocket 방식]
 * 1. ConnectWebSocket() → 서버 연결
 * 2. SendAudioToWebSocket() → 오디오 스트림 전송
 * 3. 실시간 STT → GPT → TTS 스트리밍 재생
 */
UCLASS(Blueprintable, BlueprintType)
class YISAN_API UVoiceConversationSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_SUBSYSTEM_GETTER_INLINE(UVoiceConversationSystem);

	// --- HTTP 방식 음성 대화 ---

	/** 음성 녹음 시작 */
	UFUNCTION(BlueprintCallable, Category = "Voice|Conversation")
	void StartRecording();

	/** 음성 녹음 중지 및 STT 처리 시작 */
	UFUNCTION(BlueprintCallable, Category = "Voice|Conversation")
	void StopRecording();

	/** GPT에게 텍스트로 직접 질문 (음성 녹음 없이) */
	UFUNCTION(BlueprintCallable, Category = "Voice|Conversation")
	void AskGPTDirectly(const FString& Question);

	// --- WebSocket 방식 실시간 음성 대화 ---

	/** WebSocket 연결 */
	UFUNCTION(BlueprintCallable, Category = "Voice|Conversation|WebSocket")
	void ConnectWebSocket();

	/** WebSocket 연결 해제 */
	UFUNCTION(BlueprintCallable, Category = "Voice|Conversation|WebSocket")
	void DisconnectWebSocket();

	/** WebSocket으로 오디오 전송 (실시간 스트리밍) */
	UFUNCTION(BlueprintCallable, Category = "Voice|Conversation|WebSocket")
	void SendAudioToWebSocket(const TArray<uint8>& AudioData);

	/** WebSocket으로 TTS 요청 */
	UFUNCTION(BlueprintCallable, Category = "Voice|Conversation|WebSocket")
	void RequestTTSViaWebSocket(const FString& Text);

	// --- 상태 확인 ---

	UFUNCTION(BlueprintPure, Category = "Voice|Conversation")
	bool IsRecording() const { return bIsRecording; }

	UFUNCTION(BlueprintPure, Category = "Voice|Conversation")
	bool IsProcessing() const { return bIsProcessing; }

	UFUNCTION(BlueprintPure, Category = "Voice|Conversation")
	bool IsWebSocketConnected() const;

	// --- 이벤트 ---

	UPROPERTY(BlueprintAssignable, Category = "Voice|Conversation|Events")
	FOnConversationRecordingStarted OnRecordingStarted;

	UPROPERTY(BlueprintAssignable, Category = "Voice|Conversation|Events")
	FOnConversationTranscriptionReceived OnTranscriptionReceived;

	UPROPERTY(BlueprintAssignable, Category = "Voice|Conversation|Events")
	FOnConversationGPTResponseReceived OnGPTResponseReceived;

	UPROPERTY(BlueprintAssignable, Category = "Voice|Conversation|Events")
	FOnConversationTTSStarted OnTTSStarted;

	UPROPERTY(BlueprintAssignable, Category = "Voice|Conversation|Events")
	FOnConversationCompleted OnCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Voice|Conversation|Events")
	FOnConversationError OnError;

private:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- HTTP 방식 콜백 ---
	void OnRecordingStopped(const FString& FilePath);
	void OnSTTResponse(FResponseSTT& Response, bool bSuccess);
	void OnGPTResponse(FResponseTestGPT& Response, bool bSuccess);
	void OnTTSResponse(TArray<uint8>& AudioData, bool bSuccess);

	// --- WebSocket 방식 콜백 ---
	UFUNCTION()
	void OnWebSocketConnected();
	UFUNCTION()
	void OnWebSocketTranscription(const FString& TranscribedText);
	UFUNCTION()
	void OnWebSocketAudioStart();

	// --- 컴포넌트 참조 ---
	UPROPERTY()
	TObjectPtr<class UVoiceRecordSystem> VoiceRecordSystem;

	UPROPERTY()
	TObjectPtr<class UVoiceListenSystem> VoiceListenSystem;

	// --- 상태 변수 ---
	bool bIsRecording = false;
	bool bIsProcessing = false;
	FString CurrentTranscribedText;
};
