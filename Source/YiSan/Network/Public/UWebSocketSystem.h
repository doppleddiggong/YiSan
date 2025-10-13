// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/**
 * @file UWebSocketSystem.h
 * @brief UWebSocketSystem 선언에 대한 Doxygen 주석을 제공합니다.
 */
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IWebSocket.h"
#include "Macro.h"
#include "UWebSocketSystem.generated.h"

// --- 서버 -> 클라이언트 통신을 위한 델리게이트 ---

// GPT Agent 응답
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketAgentResponse, const FString&, GPTResponse);

// TTS 오디오 스트림 시작
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketAudioStart);

// TTS 오디오 데이터 수신 (완성된 WAV 파일)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketAudioDataReceived, const TArray<uint8>&, AudioData);

// 연결 상태
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketConnectionError, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWebSocketClosed, int32, StatusCode, const FString&, Reason, bool, bWasClean);

// 서버 에러 메시지
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketError, const FString&, ErrorMessage);

// 실시간 녹음 시작 확인
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketStartRecordingAck, const FString&, Message);


UCLASS(Blueprintable, BlueprintType)
class YISAN_API UWebSocketSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_SUBSYSTEM_GETTER_INLINE(UWebSocketSystem);

	// --- 클라이언트 액션 ---

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void Connect();

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void RequestTTS(
		const FString& Text,
		const FString& VoiceName = TEXT("ko-KR-Wavenet-D"),
		float SpeakingRate = 1.0f,
		float Pitch = 0.0f,
		const FString& ReferenceIndex = TEXT("STT_00"),
		bool bUseCache = true
	);

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendAudio(const TArray<uint8>& AudioData);

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SetTTSConfig(
		const FString& VoiceName = TEXT(""),
		float SpeakingRate = -1.0f,
		float Pitch = 999.0f
	);

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendStartRecordingMessage();

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendStopRecordingMessage();

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendPing();

	UFUNCTION(BlueprintPure, Category = "WebSocket")
	bool IsConnected() const;

	// --- 서버 이벤트 ---

	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnWebSocketConnected OnConnected;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnWebSocketConnectionError OnConnectionError;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnWebSocketClosed OnClosed;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnWebSocketError OnError;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnWebSocketStartRecordingAck OnStartRecordingAck;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnWebSocketAgentResponse OnAgentResponse;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnWebSocketAudioStart OnAudioStart;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
	FOnWebSocketAudioDataReceived OnAudioDataReceived;

private:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- 네이티브 웹소켓 콜백 ---
	void OnConnected_Native();
	void OnConnectionError_Native(const FString& Error);
	void OnClosed_Native(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnMessage_Native(const FString& InMessage);
	void OnBinaryMessage_Native(const void* Data, SIZE_T Size, bool bIsLastFragment);

	TSharedPtr<IWebSocket> WebSocket;
    
	// --- 상태 관리 ---
	bool bIsExpectingAudio = false;

private:
	static void LogNetwork(const FString& Message);
};