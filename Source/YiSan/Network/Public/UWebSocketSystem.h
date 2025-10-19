// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

/// @file UWebSocketSystem.h
/// @brief 실시간 음성·텍스트 상호작용을 위한 WebSocket 서브시스템을 선언합니다.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FGPTContext.h"
#include "IWebSocket.h"
#include "Macro.h"
#include "UWebSocketSystem.generated.h"


// // --- 서버 -> 클라이언트 통신을 위한 델리게이트 ---
//
// // GPT Agent 응답
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketAgentResponse, const FString&, GPTResponse);
//
// // TTS 오디오 스트림 시작
// DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketAudioStart);
//
// // TTS 오디오 데이터 수신 (완성된 WAV 파일)
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketAudioDataReceived, const TArray<uint8>&, AudioData);
//
// // 연결 상태
// DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketConnected);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketConnectionError, const FString&, InErrorMessage);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWebSocketClosed, int32, StatusCode, const FString&, Reason, bool, bWasClean);
//
// // 서버 에러 메시지
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketError, const FString&, InErrorMessage);
//
// // 실시간 녹음 시작 확인
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketStartRecordingAck, const FString&, Message);


UCLASS(Blueprintable, BlueprintType)
class YISAN_API UWebSocketSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    DEFINE_SUBSYSTEM_GETTER_INLINE(UWebSocketSystem);

    /// @brief WebSocket 서버와 연결을 수립합니다.
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void Connect();

    /// @brief 활성화된 WebSocket 연결을 종료합니다.
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void Disconnect();

    /// @brief 실시간 음성 합성을 요청하는 메시지를 전송합니다.
    /// @param Text [in] 합성할 문장입니다.
    /// @param VoiceName [in] 음성 프로필 이름입니다.
    /// @param SpeakingRate [in] 발화 속도 배수입니다.
    /// @param Pitch [in] 피치 조절 값입니다.
    /// @param ReferenceIndex [in] 캐시 키로 사용하는 참조 문자열입니다.
    /// @param bUseCache [in] 서버 캐시 사용 여부입니다.
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void RequestTTS(
        const FString& Text,
        const FString& VoiceName = TEXT("ko-KR-Wavenet-D"),
        float SpeakingRate = 1.0f,
        float Pitch = 0.0f,
        const FString& ReferenceIndex = TEXT("STT_00"),
        bool bUseCache = true
    );

    /// @brief 실시간 음성 스트리밍 데이터를 서버로 전송합니다.
    /// @param AudioData [in] 전송할 PCM/WAV 바이트 배열입니다.
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void SendAudio(const TArray<uint8>& AudioData);

    /// @brief 서버에 기본 음성 합성 파라미터를 업데이트합니다.
    /// @param VoiceName [in] 변경할 음성 프로필 이름입니다.
    /// @param SpeakingRate [in] 기본 발화 속도 배수입니다.
    /// @param Pitch [in] 기본 피치 값입니다.
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void SetTTSConfig(
        const FString& VoiceName = TEXT(""),
        float SpeakingRate = -1.0f,
        float Pitch = 999.0f
    );

    /// @brief 서버에 녹음 시작 신호를 보냅니다.
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void SendStartRecordingMessage();

    /// @brief 서버에 녹음 종료 신호를 전송합니다.
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void SendStopRecordingMessage();

    /// @brief 연결 상태를 유지하기 위해 Ping 메시지를 보냅니다.
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void SendPing();

    /// @brief 플레이어 주변 공간 정보를 실시간으로 동기화합니다.
    /// @param Context [in] 위치·시선 정보를 포함한 GPT 컨텍스트입니다.
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
    void SendSpatialContext(const FGPTContext& Context);

    /// @brief 현재 WebSocket 연결 상태를 조회합니다.
    UFUNCTION(BlueprintPure, Category = "WebSocket")
    bool IsConnected() const;

    // --- 서버 이벤트 ---

    // UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
    // FOnWebSocketConnected OnConnected;
    //
    // UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
    // FOnWebSocketConnectionError OnConnectionError;
    //
    // UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
    // FOnWebSocketClosed OnClosed;

    // UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
    // FOnWebSocketError OnSocketError;

    // UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
    // FOnWebSocketStartRecordingAck OnStartRecordingAck;
    //
    // UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
    // FOnWebSocketAgentResponse OnAgentResponse;
    //
    // UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
    // FOnWebSocketAudioStart OnAudioStart;
    //
    // UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
    // FOnWebSocketAudioDataReceived OnAudioDataReceived;

private:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /// @brief WebSocket 연결 완료 콜백입니다.
    void OnConnected_Native();

    /// @brief 연결 오류 발생 시 호출되는 콜백입니다.
    /// @param InErrorMessage [in] 서버에서 전달된 에러 메시지입니다.
    void OnConnectionError_Native(const FString& InErrorMessage);

    /// @brief 연결 종료 이벤트를 처리합니다.
    /// @param StatusCode [in] WebSocket 상태 코드입니다.
    /// @param Reason [in] 종료 사유입니다.
    /// @param bWasClean [in] 정상 종료 여부입니다.
    void OnClosed_Native(int32 StatusCode, const FString& Reason, bool bWasClean);

    /// @brief 텍스트 메시지를 수신했을 때 호출됩니다.
    /// @param InMessage [in] 서버에서 전달한 JSON/문자열입니다.
    void OnMessage_Native(const FString& InMessage);

    /// @brief 바이너리 메시지를 수신했을 때 호출됩니다.
    /// @param Data [in] 수신 버퍼 시작 주소입니다.
    /// @param Size [in] 버퍼 크기입니다.
    /// @param bIsLastFragment [in] 해당 프레임이 마지막 조각인지 여부입니다.
    void OnBinaryMessage_Native(const void* Data, SIZE_T Size, bool bIsLastFragment);

    /// @brief 활성 WebSocket 인스턴스입니다.
    TSharedPtr<IWebSocket> WebSocket;

    /// @brief 서버에서 오디오 스트림 시작을 예상하는지 여부입니다.
    bool bIsExpectingAudio = false;

private:
    /// @brief WebSocket 관련 공통 로그 메시지를 출력합니다.
    static void LogNetwork(const FString& Message);
};
