// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UVoiceConversationSystem.h"

#include "GameLogging.h"
#include "UBroadcastManger.h"
#include "UHttpNetworkSystem.h"
#include "UWebSocketSystem.h"
#include "UVoiceListenSystem.h"
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

			VoiceListenSystem = NewObject<UVoiceListenSystem>(GameInstance);
			VoiceListenSystem->RegisterComponent();
			VoiceListenSystem->InitSystem();
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
		// {
		// 	const FName FUNCTION_NAME = GET_FUNCTION_NAME_CHECKED(UVoiceConversationSystem, OnWebSocketTranscription);
		// 	if (!WebSocketSystem->OnTranscriptionReceived.Contains(this, FUNCTION_NAME))
		// 		WebSocketSystem->OnTranscriptionReceived.AddDynamic(this, &UVoiceConversationSystem::OnWebSocketTranscription);
		// }

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

	if (VoiceListenSystem && VoiceListenSystem->IsValidLowLevel())
	{
		VoiceListenSystem->DestroyComponent();
	}

	if (StreamingRecordSystem && StreamingRecordSystem->IsValidLowLevel())
	{
		StreamingRecordSystem->DestroyComponent();
	}

	if (UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld()))
	{
		WebSocketSystem->OnConnected.RemoveAll(this);
		WebSocketSystem->OnTranscriptionReceived.RemoveAll(this);
		WebSocketSystem->OnAudioStart.RemoveAll(this);
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

    const FName AckDelegateName = GET_FUNCTION_NAME_CHECKED(UVoiceConversationSystem, OnWebSocketStartRecordingAck);
    if (!WebSocketSystem->OnStartRecordingAck.Contains(this, AckDelegateName))
    {
        PRINTLOG(TEXT("Binding OnStartRecordingAck delegate before starting streaming."));
        WebSocketSystem->OnStartRecordingAck.AddDynamic(this, &UVoiceConversationSystem::OnWebSocketStartRecordingAck);
    }

    const FName ChunkDelegateName = GET_FUNCTION_NAME_CHECKED(UVoiceConversationSystem, OnAudioChunkReady);
    if (!StreamingRecordSystem->OnAudioChunkReady.Contains(this, ChunkDelegateName))
    {
        StreamingRecordSystem->OnAudioChunkReady.AddDynamic(this, &UVoiceConversationSystem::OnAudioChunkReady);
    }

    bIsStreamingActive = true; // Set to true immediately to prevent multiple calls
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
    StreamingRecordSystem->StopRecording();
    bIsRecording = false;
    bIsStreamingActive = false;

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

	if (UBroadcastManger* EventManager = UBroadcastManger::Get(this))
		EventManager->SendToastMessage(TEXT("실시간 음성 서버에 연결되었습니다."));
}
//
// void UVoiceConversationSystem::OnWebSocketTranscription(const FString& TranscribedText)
// {
// 	if (TranscribedText.IsEmpty())
// 	{
// 		PRINTLOG(TEXT("[VoiceConversation] WebSocket STT: (empty)"));
// 		return;
// 	}
//
// 	PRINTLOG(TEXT("[VoiceConversation] WebSocket STT: %s"), *TranscribedText);
//
// 	if (UBroadcastManger* EventManager = UBroadcastManger::Get(this))
// 	{
// 		EventManager->SendToastMessage(TranscribedText);
// 	}
// }

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
	SendStreamAudio(PcmData);
}

void UVoiceConversationSystem::OnWebSocketStartRecordingAck(const FString& Message)
{
    if (!StreamingRecordSystem)
    {
        PRINTLOG(TEXT("StreamingRecordSystem is not initialized."));
        return;
    }

	// Start actual recording
    bIsRecording = true;
    StreamingRecordSystem->StartRecording();
    PRINTLOG(TEXT("[VoiceConversation] Stream recording started by server ack: %s"), *Message);
}