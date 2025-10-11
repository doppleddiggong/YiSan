// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UVoiceConversationSystem.h"

#include "GameLogging.h"
#include "UBroadcastManger.h"
#include "UHttpNetworkSystem.h"
#include "UVoiceRecordSystem.h"
#include "UVoiceListenSystem.h"
#include "UWebSocketSystem.h"
#include "GameFramework/GameModeBase.h"

// --- Subsystem Lifecycle ---

void UVoiceConversationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// VoiceRecordSystem 생성
	if (UWorld* World = GetWorld())
	{
		// GameInstance를 Owner로 사용 (클라이언트/서버 모두 작동)
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			VoiceRecordSystem = NewObject<UVoiceRecordSystem>(GameInstance);
			VoiceRecordSystem->RegisterComponent();
			// VoiceRecordSystem->OnRecordingStopped.AddDynamic(this, &UVoiceConversationSystem::OnRecordingStopped);

			VoiceListenSystem = NewObject<UVoiceListenSystem>(GameInstance);
			VoiceListenSystem->RegisterComponent();
			VoiceListenSystem->InitSystem();
		}
	}

	// WebSocket 이벤트 바인딩
	if (UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld()))
	{
		WebSocketSystem->OnConnected.AddDynamic(this, &UVoiceConversationSystem::OnWebSocketConnected);
		WebSocketSystem->OnTranscriptionReceived.AddDynamic(this, &UVoiceConversationSystem::OnWebSocketTranscription);
		WebSocketSystem->OnAudioStart.AddDynamic(this, &UVoiceConversationSystem::OnWebSocketAudioStart);
	}

	PRINTLOG(TEXT("[VoiceConversation] System initialized."));
}

void UVoiceConversationSystem::Deinitialize()
{
	if (VoiceRecordSystem && VoiceRecordSystem->IsValidLowLevel())
	{
		// VoiceRecordSystem->OnRecordingStopped.RemoveAll(this);
		VoiceRecordSystem->DestroyComponent();
	}

	if (VoiceListenSystem && VoiceListenSystem->IsValidLowLevel())
	{
		VoiceListenSystem->DestroyComponent();
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
		// OnError.Broadcast(TEXT("VoiceRecordSystem이 초기화되지 않았습니다."));
		return;
	}

	PRINT_STRING(TEXT("Cmd_RecordStart_Implementation"));

	
	bIsRecording = true;
	VoiceRecordSystem->RecordStart();
	// OnRecordingStarted.Broadcast();

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
	auto FilePath = VoiceRecordSystem->RecordStop();

	PRINTLOG( TEXT("[VoiceConversation] Recording stopped. Processing...") );

	OnRecordingStopped( FilePath );
}

void UVoiceConversationSystem::AskGPTDirectly(const FString& Question)
{
	if (bIsProcessing)
	{
		PRINTLOG( TEXT("이미 처리 중입니다."));
		// OnError.Broadcast(TEXT("이미 처리 중입니다."));
		return;
	}

	bIsProcessing = true;
	// CurrentTranscribedText = Question;
	// OnTranscriptionReceived.Broadcast(Question);

	// GPT 요청
	UHttpNetworkSystem* HttpSystem = UHttpNetworkSystem::Get(GetWorld());
	if (!HttpSystem)
	{
		// OnError.Broadcast(TEXT("HttpSystem을 찾을 수 없습니다."));
		bIsProcessing = false;
		return;
	}

	HttpSystem->RequestTestGPT(Question, FResponseTestGPTDelegate::CreateUObject(
		this, &UVoiceConversationSystem::OnGPTResponse
	));
}

void UVoiceConversationSystem::OnRecordingStopped(const FString& FilePath)
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
		// OnError.Broadcast(TEXT("HttpSystem을 찾을 수 없습니다."));
		bIsProcessing = false;
		return;
	}

	HttpSystem->RequestTestSTT(FilePath, FResponseTestSTTDelegate::CreateUObject(
		this, &UVoiceConversationSystem::OnResponseTestSTT
	));
}

void UVoiceConversationSystem::OnResponseTestSTT(FResponseTestSTT& Response, bool bSuccess)
{
	bIsProcessing = false;

	if (bSuccess)
	{
		if (auto EventManager = UBroadcastManger::Get(this))
			EventManager->SendToastMessage(Response.text);
	}
	else
	{
		PRINTLOG( TEXT("--- Network Response Received (FAIL) ---"));
	}
}

void UVoiceConversationSystem::OnGPTResponse(FResponseTestGPT& Response, bool bSuccess)
{
	if (!bSuccess || Response.response.IsEmpty())
	{
		PRINTLOG( TEXT("GPT 처리에 실패했습니다."));
		bIsProcessing = false;
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[VoiceConversation] GPT: %s"), *Response.response);

	// TTS 요청
	UHttpNetworkSystem* HttpSystem = UHttpNetworkSystem::Get(GetWorld());
	if (!HttpSystem)
	{
		PRINTLOG( TEXT("HttpSystem을 찾을 수 없습니다."));
		bIsProcessing = false;
		return;
	}

	HttpSystem->RequestTestTTS(Response.response, 0.88f, -3.0f, TEXT("ko-KR-Wavenet-D"),
		FResponseTestTTSDelegate::CreateUObject( this, &UVoiceConversationSystem::OnResponseTestTTS ));
}

void UVoiceConversationSystem::OnResponseTestTTS(FResponseTestTTS& Response, bool bSuccess)
{
	if (bSuccess)
	{
		if (Response.audio_data.Num() == 0)
		{
			PRINTLOG(TEXT("TTS 응답은 성공했으나 audio_data가 비어있습니다."));
			return;
		}

		PRINTLOG(TEXT("TTS 응답 수신: %d bytes"), Response.audio_data.Num());

		VoiceListenSystem->HandleTTSOutput(Response.audio_data);
	}
	else
	{
		PRINTLOG( TEXT("--- Network Response Received (FAIL) ---"));
	}
}

// --- WebSocket 방식 실시간 음성 대화 ---

void UVoiceConversationSystem::ConnectWebSocket()
{
	UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld());
	if (!WebSocketSystem)
	{
		PRINTLOG( TEXT("WebSocketSystem을 찾을 수 없습니다."));
		return;
	}

	WebSocketSystem->Connect();
}

void UVoiceConversationSystem::DisconnectWebSocket()
{
	UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld());
	if (!WebSocketSystem)
	{
		return;
	}

	WebSocketSystem->Disconnect();
}

void UVoiceConversationSystem::SendAudioToWebSocket(const TArray<uint8>& AudioData)
{
	UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld());
	if (!WebSocketSystem || !WebSocketSystem->IsConnected())
	{
		PRINTLOG( TEXT("WebSocket이 연결되지 않았습니다."));
		return;
	}

	WebSocketSystem->SendAudio(AudioData);
}

void UVoiceConversationSystem::RequestTTSViaWebSocket(const FString& Text)
{
	UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld());
	if (!WebSocketSystem || !WebSocketSystem->IsConnected())
	{
		PRINTLOG( TEXT("WebSocket이 연결되지 않았습니다."));
		return;
	}

	WebSocketSystem->RequestTTS(Text, TEXT("ko-KR-Wavenet-D"), 1.0f, 0.0f, TEXT("STT_00"), true);
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
}

void UVoiceConversationSystem::OnWebSocketTranscription(const FString& TranscribedText)
{
	PRINTLOG( TEXT("[VoiceConversation] WebSocket STT: %s"), *TranscribedText );
}

void UVoiceConversationSystem::OnWebSocketAudioStart()
{
	PRINTLOG( TEXT("[VoiceConversation] WebSocket TTS streaming started."));
}
