// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UVoiceConversationSystem.h"

#include "GameLogging.h"
#include "UBroadcastManger.h"
#include "UHttpNetworkSystem.h"
#include "UWebSocketSystem.h"
#include "UVoiceRecordSystem.h"
#include "UStreamingRecordSystem.h"
#include "UVoiceFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWaveProcedural.h"

// --- Subsystem Lifecycle ---

void UVoiceConversationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<UWebSocketSystem>();

	// VoiceRecordSystem 생성 (ASK API용)
	if (UWorld* World = GetWorld())
	{
		// GameInstance를 Owner로 사용 (클라이언트/서버 모두 작동)
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			VoiceRecordSystem = NewObject<UVoiceRecordSystem>(GameInstance);
			VoiceRecordSystem->RegisterComponent();

			StreamingRecordSystem = NewObject<UStreamingRecordSystem>(GameInstance);
			StreamingRecordSystem->RegisterComponent();

			PRINTLOG(TEXT("[VoiceConversation] Voice systems initialized"));
		}
	}

	// WebSocket 이벤트 바인딩
	if (UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld()))
	{
		{
			const FName FUNCTION_NAME = GET_FUNCTION_NAME_CHECKED(UVoiceConversationSystem, OnWebSocketConnected);
			if (!WebSocketSystem->OnConnected.Contains(this, FUNCTION_NAME))
				WebSocketSystem->OnConnected.AddDynamic(this, &UVoiceConversationSystem::OnWebSocketConnected);
		}

		{
			const FName FUNCTION_NAME = GET_FUNCTION_NAME_CHECKED(UVoiceConversationSystem, OnWebSocketAudioStart);
			if (!WebSocketSystem->OnAudioStart.Contains(this, FUNCTION_NAME))
				WebSocketSystem->OnAudioStart.AddDynamic(this, &UVoiceConversationSystem::OnWebSocketAudioStart);
		}
		{
			const FName FUNCTION_NAME = GET_FUNCTION_NAME_CHECKED(UVoiceConversationSystem, OnWebSocketStartRecordingAck);
			if (!WebSocketSystem->OnStartRecordingAck.Contains(this, FUNCTION_NAME))
				WebSocketSystem->OnStartRecordingAck.AddDynamic(this, &UVoiceConversationSystem::OnWebSocketStartRecordingAck);
		}
		{
			const FName FUNCTION_NAME = GET_FUNCTION_NAME_CHECKED(UVoiceConversationSystem, OnWebSocketAudioDataReceived);
			if (!WebSocketSystem->OnAudioDataReceived.Contains(this, FUNCTION_NAME))
				WebSocketSystem->OnAudioDataReceived.AddDynamic(this, &UVoiceConversationSystem::OnWebSocketAudioDataReceived);
		}
	}
	else
	{
		PRINTLOG(TEXT("[VoiceConversation] WebSocketSystem was not ready during Initialize. Binding will be retried when streaming starts."));
	}

	PRINTLOG(TEXT("[VoiceConversation] System initialized."));
}

void UVoiceConversationSystem::Deinitialize()
{
	if (VoiceRecordSystem && VoiceRecordSystem->IsValidLowLevel())
	{
		VoiceRecordSystem->DestroyComponent();
	}

	if (StreamingRecordSystem && StreamingRecordSystem->IsValidLowLevel())
	{
		StreamingRecordSystem->DestroyComponent();
	}

	if (UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld()))
	{
		WebSocketSystem->OnConnected.RemoveAll(this);
		WebSocketSystem->OnAudioStart.RemoveAll(this);
		WebSocketSystem->OnAudioDataReceived.RemoveAll(this);
	}

	Super::Deinitialize();
}

// --- HTTP 방식 음성 대화 ---

void UVoiceConversationSystem::StartRecording()
{
	if (bIsRecording || bIsProcessing)
	{
		PRINTLOG( TEXT("[VoiceConversation] Already recording or processing."));
		return;
	}

	if (!VoiceRecordSystem)
	{
		PRINTLOG( TEXT("VoiceRecordSystem이 초기화되지 않았습니다."));
		return;
	}
	
	bIsRecording = true;
	VoiceRecordSystem->RecordStart();

	PRINTLOG( TEXT("[VoiceConversation] Recording started."));
}

void UVoiceConversationSystem::StopRecording()
{
	if (!bIsRecording)
	{
		PRINTLOG( TEXT("[VoiceConversation] Not currently recording."));
		return;
	}

	bIsRecording = false;
	bIsProcessing = true;

	PRINTLOG( TEXT("[VoiceConversation] Recording stopped. Processing...") );

	SendAskFromCaptureData( VoiceRecordSystem->RecordStop() );
}

void UVoiceConversationSystem::SendAskFromCaptureData(const FString& FilePath)
{
	PRINTLOG( TEXT("[VoiceConversation] Recording saved to: %s"), *FilePath);

	if (FilePath.IsEmpty())
	{
		PRINTLOG( TEXT("[VoiceConversation] FilePath is Empty") );
		return;
	}
	
	// STT 요청
	UHttpNetworkSystem* HttpSystem = UHttpNetworkSystem::Get(GetWorld());
	if (!HttpSystem)
	{
		PRINTLOG( TEXT("HttpSystem을 찾을 수 없습니다."));
		bIsProcessing = false;
		return;
	}

	HttpSystem->RequestAsk(FilePath, FResponseAskDelegate::CreateUObject(
		this, &UVoiceConversationSystem::OnResponseAsk
	));
}

void UVoiceConversationSystem::OnResponseAsk(FResponseAsk& Response, bool bSuccess)
{
	bIsProcessing = false;

	if (bSuccess)
	{
		if (auto EventManager = UBroadcastManger::Get(this))
			EventManager->SendToastMessage(Response.gpt_response_text);
		
		auto SoundWave = UVoiceFunctionLibrary::CreateProceduralSoundWaveFromWavData(Response.audio_data);
		if ( IsValid(SoundWave))
			UGameplayStatics::PlaySound2D(this, SoundWave);
	}
	else
	{
		PRINTLOG( TEXT("--- Network Response Received (FAIL) ---"));
	}
}


void UVoiceConversationSystem::StartStreaming()
{
    if (bIsRecording)
    {
        NETWORK_LOG(TEXT("[VoiceConversation] Already recording."));
        return;
    }

    if (!StreamingRecordSystem)
    {
		NETWORK_LOG( TEXT("StreamingRecordSystem is not initialized."));
        return;
    }

    UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld());
    if (!WebSocketSystem)
    {
        PRINTLOG(TEXT("[VoiceConversation] WebSocketSystem is not initialized."));
        bIsRecording = false;
        return;
    }

    if (!WebSocketSystem->IsConnected())
    {
        PRINTLOG(TEXT("[VoiceConversation] Cannot start StartStream recording. WebSocket not connected."));
        bIsRecording = false;
        return;
    }

	{
		const FName FUNCTION_NAME = GET_FUNCTION_NAME_CHECKED(UVoiceConversationSystem, OnWebSocketStartRecordingAck);
    	if (!WebSocketSystem->OnStartRecordingAck.Contains(this, FUNCTION_NAME))
    		WebSocketSystem->OnStartRecordingAck.AddDynamic(this, &UVoiceConversationSystem::OnWebSocketStartRecordingAck);
	}

	{
		const FName FUNCTION_NAME = GET_FUNCTION_NAME_CHECKED(UVoiceConversationSystem, OnAudioChunkReady);
    	if (!StreamingRecordSystem->OnAudioChunkReady.Contains(this, FUNCTION_NAME))
    	{
    		StreamingRecordSystem->OnAudioChunkReady.AddDynamic(this, &UVoiceConversationSystem::OnAudioChunkReady);
    	}
	}

	// 버퍼 초기화 및 버퍼링 모드 활성화
	AudioBuffer.Empty();
	bIsWaitingForServerAck = true;
	bIsStreamingActive = true;
	bIsRecording = true;

	// 즉시 녹음 시작 (서버 ACK를 기다리지 않음)
	StreamingRecordSystem->StartStreaming();
	PRINTLOG(TEXT("[VoiceConversation] Recording started immediately (buffering until server ACK)"));

	// 서버에 시작 메시지 전송
    WebSocketSystem->SendStartRecordingMessage();
}

void UVoiceConversationSystem::StopStreaming()
{
    if (!bIsStreamingActive)
    {
    	PRINTLOG(TEXT("StreamingRecordSystem is not Streaming Active"));
        return;
    }

    // Stop the audio capture
    StreamingRecordSystem->StopStreaming();
    bIsRecording = false;
    bIsStreamingActive = false;
	bIsWaitingForServerAck = false;

	// 버퍼에 남은 데이터 전송 (녹음 중지 직전에 쌓인 데이터)
	if (AudioBuffer.Num() > 0)
	{
		PRINTLOG(TEXT("[VoiceConversation] Sending remaining buffered audio chunks (%d chunks) before stopping"), AudioBuffer.Num());
		for (const TArray<uint8>& BufferedChunk : AudioBuffer)
		{
			SendStreamAudio(BufferedChunk);
		}
		AudioBuffer.Empty();
	}

    // Unbind the delegate
    if(StreamingRecordSystem->OnAudioChunkReady.IsBound())
    {
    	StreamingRecordSystem->OnAudioChunkReady.RemoveDynamic(this, &UVoiceConversationSystem::OnAudioChunkReady);
    }

    // Notify the server
    if (UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld()))
    {
        if (WebSocketSystem->IsConnected())
        {
            WebSocketSystem->SendStopRecordingMessage();
        }
    }

    PRINTLOG(TEXT("[VoiceConversation] StreamingRecordSystem recording stopped."));
}

void UVoiceConversationSystem::SendStreamAudio(const TArray<uint8>& AudioData)
{
	UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld());
	if (!WebSocketSystem || !WebSocketSystem->IsConnected())
	{
		PRINTLOG( TEXT("WebSocket이 연결되지 않았습니다."));
		return;
	}

	// PRINTLOG(TEXT("[VoiceConversation] Sending audio chunk to WebSocket (%d bytes)"), AudioData.Num());
	WebSocketSystem->SendAudio(AudioData);
}

bool UVoiceConversationSystem::IsWebSocketConnected() const
{
	UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld());
	return WebSocketSystem && WebSocketSystem->IsConnected();
}

// --- WebSocket 방식 콜백 ---
void UVoiceConversationSystem::OnWebSocketConnected()
{
	PRINTLOG(TEXT("[VoiceConversation] WebSocket connected."));

	bIsStreamingActive = false;
	bIsRecording = false;
	bIsProcessing = false;
	bIsWaitingForServerAck = false;
	AudioBuffer.Empty();

	if (UBroadcastManger* EventManager = UBroadcastManger::Get(this))
		EventManager->SendToastMessage(TEXT("실시간 음성 서버에 연결되었습니다."));
}

void UVoiceConversationSystem::OnWebSocketAudioStart()
{
	PRINTLOG(TEXT("[VoiceConversation] WebSocket TTS streaming started."));

	if (UBroadcastManger* EventManager = UBroadcastManger::Get(this))
	{
		EventManager->SendToastMessage(TEXT("AI 음성 응답을 재생합니다."));
	}
}

void UVoiceConversationSystem::OnAudioChunkReady(const TArray<uint8>& PcmData)
{
	if (bIsWaitingForServerAck)
	{
		// 서버 ACK 전: 버퍼에 쌓기
		AudioBuffer.Add(PcmData);
		// PRINTLOG(TEXT("[VoiceConversation] Buffering audio chunk (%d bytes, total: %d chunks)"), PcmData.Num(), AudioBuffer.Num());
	}
	else
	{
		// 서버 ACK 후: 즉시 전송
		SendStreamAudio(PcmData);
	}
}

void UVoiceConversationSystem::OnWebSocketStartRecordingAck(const FString& Message)
{
	PRINTLOG(TEXT("[VoiceConversation] Server ACK received: %s"), *Message);

	// 버퍼링 모드 해제
	bIsWaitingForServerAck = false;

	// 버퍼에 쌓인 데이터 전송
	if (AudioBuffer.Num() > 0)
	{
		PRINTLOG(TEXT("[VoiceConversation] Sending buffered audio chunks (%d chunks)"), AudioBuffer.Num());

		for (const TArray<uint8>& BufferedChunk : AudioBuffer)
		{
			SendStreamAudio(BufferedChunk);
		}

		AudioBuffer.Empty();
		PRINTLOG(TEXT("[VoiceConversation] All buffered audio sent"));
	}
	else
	{
		PRINTLOG(TEXT("[VoiceConversation] No buffered audio to send"));
	}
}

void UVoiceConversationSystem::OnWebSocketAudioDataReceived(const TArray<uint8>& AudioData)
{
	PRINTLOG(TEXT("[VoiceConversation] Received audio data: %d bytes"), AudioData.Num());

	if (AudioData.Num() < 44)
	{
		PRINTLOG(TEXT("[VoiceConversation] Audio data too small, ignoring"));
		return;
	}

	// WAV 파일 생성 및 재생
	USoundWaveProcedural* SoundWave = UVoiceFunctionLibrary::CreateProceduralSoundWaveFromWavData(AudioData);
	if (SoundWave)
	{
		UGameplayStatics::PlaySound2D(this, SoundWave);
		PRINTLOG(TEXT("[VoiceConversation] Audio playback started"));
	}
	else
	{
		PRINTLOG(TEXT("[VoiceConversation] Failed to create sound wave"));
	}
}