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

	/** 음성 녹음 중지 및 ASK 처리 시작 */
	UFUNCTION(BlueprintCallable, Category = "Voice|Conversation")
	void StopRecording();

	// --- WebSocket 방식 실시간 음성 대화 ---
	/** 실시간 음성 스트리밍 시작 */
	UFUNCTION(BlueprintCallable, Category = "Voice|Conversation|WebSocket")
	void StartStreaming();

	/** 실시간 음성 스트리밍 중지 */
	UFUNCTION(BlueprintCallable, Category = "Voice|Conversation|WebSocket")
	void StopStreaming();

	/** WebSocket으로 오디오 전송 (실시간 스트리밍) */
	UFUNCTION(BlueprintCallable, Category = "Voice|Conversation|WebSocket")
	void SendStreamAudio(const TArray<uint8>& AudioData);

	// --- 상태 확인 ---
	UFUNCTION(BlueprintPure, Category = "Voice|Conversation")
	bool IsRecording() const { return bIsRecording; }

	UFUNCTION(BlueprintPure, Category = "Voice|Conversation")
	bool IsProcessing() const { return bIsProcessing; }

	UFUNCTION(BlueprintPure, Category = "Voice|Conversation")
	bool IsWebSocketConnected() const;

private:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- HTTP 방식 콜백 ---
	UFUNCTION()
	void SendAskFromCaptureData(const FString& FilePath);
	
	void OnResponseAsk(FResponseAsk& Response, bool bSuccess);

	// --- WebSocket 방식 콜백 ---
	UFUNCTION()
	void OnAudioChunkReady(const TArray<uint8>& PcmData);
	UFUNCTION()
	void OnWebSocketConnected();
	UFUNCTION()
	void OnWebSocketAudioStart();
	UFUNCTION()
	void OnWebSocketStartRecordingAck(const FString& Message);
	UFUNCTION()
	void OnWebSocketAudioDataReceived(const TArray<uint8>& AudioData);

	// --- 컴포넌트 참조 ---
	UPROPERTY()
	TObjectPtr<class UVoiceRecordSystem> VoiceRecordSystem;

	UPROPERTY()
	TObjectPtr<class UStreamingRecordSystem> StreamingRecordSystem;

	// --- 상태 변수 ---
	bool bIsRecording = false;
	bool bIsProcessing = false;
    bool bIsStreamingActive = false;
	bool bIsWaitingForServerAck = false;

	// 서버 ACK 전까지 오디오 데이터를 버퍼링
	TArray<TArray<uint8>> AudioBuffer;
};
