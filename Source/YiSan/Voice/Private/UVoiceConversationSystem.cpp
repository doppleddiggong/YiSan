// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UVoiceConversationSystem.h"

#include "UHttpNetworkSystem.h"
#include "UVoiceRecordSystem.h"
#include "UVoiceListenSystem.h"
#include "UWebSocketSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWaveProcedural.h"
#include "GameFramework/GameModeBase.h"

// --- Subsystem Lifecycle ---

void UVoiceConversationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// VoiceRecordSystem 생성
	if (UWorld* World = GetWorld())
	{
		if (AGameModeBase* GameMode = World->GetAuthGameMode())
		{
			VoiceRecordSystem = NewObject<UVoiceRecordSystem>(GameMode);
			VoiceRecordSystem->RegisterComponent();
			VoiceRecordSystem->OnRecordingStopped.AddDynamic(this, &UVoiceConversationSystem::OnRecordingStopped);

			VoiceListenSystem = NewObject<UVoiceListenSystem>(GameMode);
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

	UE_LOG(LogTemp, Log, TEXT("[VoiceConversation] System initialized."));
}

void UVoiceConversationSystem::Deinitialize()
{
	if (VoiceRecordSystem && VoiceRecordSystem->IsValidLowLevel())
	{
		VoiceRecordSystem->OnRecordingStopped.RemoveAll(this);
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
		UE_LOG(LogTemp, Warning, TEXT("[VoiceConversation] Already recording or processing."));
		return;
	}

	if (!VoiceRecordSystem)
	{
		OnError.Broadcast(TEXT("VoiceRecordSystem이 초기화되지 않았습니다."));
		return;
	}

	bIsRecording = true;
	VoiceRecordSystem->RecordStart();
	OnRecordingStarted.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[VoiceConversation] Recording started."));
}

void UVoiceConversationSystem::StopRecording()
{
	if (!bIsRecording)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceConversation] Not currently recording."));
		return;
	}

	bIsRecording = false;
	bIsProcessing = true;
	VoiceRecordSystem->RecordStop();

	UE_LOG(LogTemp, Log, TEXT("[VoiceConversation] Recording stopped. Processing..."));
}

void UVoiceConversationSystem::AskGPTDirectly(const FString& Question)
{
	if (bIsProcessing)
	{
		OnError.Broadcast(TEXT("이미 처리 중입니다."));
		return;
	}

	bIsProcessing = true;
	CurrentTranscribedText = Question;
	OnTranscriptionReceived.Broadcast(Question);

	// GPT 요청
	UHttpNetworkSystem* HttpSystem = UHttpNetworkSystem::Get(GetWorld());
	if (!HttpSystem)
	{
		OnError.Broadcast(TEXT("HttpSystem을 찾을 수 없습니다."));
		bIsProcessing = false;
		return;
	}

	HttpSystem->RequestTestGPT(Question, FResponseTestGPTDelegate::CreateUObject(
		this, &UVoiceConversationSystem::OnGPTResponse
	));
}

void UVoiceConversationSystem::OnRecordingStopped(const FString& FilePath)
{
	UE_LOG(LogTemp, Log, TEXT("[VoiceConversation] Recording saved to: %s"), *FilePath);

	// STT 요청
	UHttpNetworkSystem* HttpSystem = UHttpNetworkSystem::Get(GetWorld());
	if (!HttpSystem)
	{
		OnError.Broadcast(TEXT("HttpSystem을 찾을 수 없습니다."));
		bIsProcessing = false;
		return;
	}

	HttpSystem->RequestSTT(FilePath, FResponseSTTDelegate::CreateUObject(
		this, &UVoiceConversationSystem::OnSTTResponse
	));
}

void UVoiceConversationSystem::OnSTTResponse(FResponseSTT& Response, bool bSuccess)
{
	if (!bSuccess || Response.text.IsEmpty())
	{
		OnError.Broadcast(TEXT("STT 처리에 실패했습니다."));
		bIsProcessing = false;
		return;
	}

	CurrentTranscribedText = Response.text;
	OnTranscriptionReceived.Broadcast(Response.text);

	UE_LOG(LogTemp, Log, TEXT("[VoiceConversation] STT: %s"), *Response.text);

	// GPT 요청
	UHttpNetworkSystem* HttpSystem = UHttpNetworkSystem::Get(GetWorld());
	if (!HttpSystem)
	{
		OnError.Broadcast(TEXT("HttpSystem을 찾을 수 없습니다."));
		bIsProcessing = false;
		return;
	}

	HttpSystem->RequestTestGPT(Response.text, FResponseTestGPTDelegate::CreateUObject(
		this, &UVoiceConversationSystem::OnGPTResponse
	));
}

void UVoiceConversationSystem::OnGPTResponse(FResponseTestGPT& Response, bool bSuccess)
{
	if (!bSuccess || Response.response.IsEmpty())
	{
		OnError.Broadcast(TEXT("GPT 처리에 실패했습니다."));
		bIsProcessing = false;
		return;
	}

	OnGPTResponseReceived.Broadcast(Response.response);

	UE_LOG(LogTemp, Log, TEXT("[VoiceConversation] GPT: %s"), *Response.response);

	// TTS 요청
	UHttpNetworkSystem* HttpSystem = UHttpNetworkSystem::Get(GetWorld());
	if (!HttpSystem)
	{
		OnError.Broadcast(TEXT("HttpSystem을 찾을 수 없습니다."));
		bIsProcessing = false;
		return;
	}

	OnTTSStarted.Broadcast();

	HttpSystem->RequestTestTTS(Response.response, 0.88f, -3.0f, FResponseTestTTSDelegate::CreateUObject(
		this, &UVoiceConversationSystem::OnTTSResponse
	));
}

void UVoiceConversationSystem::OnTTSResponse(TArray<uint8>& AudioData, bool bSuccess)
{
	bIsProcessing = false;

	if (!bSuccess || AudioData.Num() == 0)
	{
		OnError.Broadcast(TEXT("TTS 처리에 실패했습니다."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[VoiceConversation] TTS: Received %d bytes of audio data."), AudioData.Num());

	// TODO: 오디오 재생 기능 구현
	// USoundWave를 생성하고 AudioComponent로 재생하는 로직 추가 필요

	OnCompleted.Broadcast();
}

// --- WebSocket 방식 실시간 음성 대화 ---

void UVoiceConversationSystem::ConnectWebSocket()
{
	UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld());
	if (!WebSocketSystem)
	{
		OnError.Broadcast(TEXT("WebSocketSystem을 찾을 수 없습니다."));
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
		OnError.Broadcast(TEXT("WebSocket이 연결되지 않았습니다."));
		return;
	}

	WebSocketSystem->SendAudio(AudioData);
}

void UVoiceConversationSystem::RequestTTSViaWebSocket(const FString& Text)
{
	UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld());
	if (!WebSocketSystem || !WebSocketSystem->IsConnected())
	{
		OnError.Broadcast(TEXT("WebSocket이 연결되지 않았습니다."));
		return;
	}

	WebSocketSystem->RequestTTS(Text, TEXT("STT_00"), true);
}

bool UVoiceConversationSystem::IsWebSocketConnected() const
{
	UWebSocketSystem* WebSocketSystem = UWebSocketSystem::Get(GetWorld());
	return WebSocketSystem && WebSocketSystem->IsConnected();
}

// --- WebSocket 방식 콜백 ---

void UVoiceConversationSystem::OnWebSocketConnected()
{
	UE_LOG(LogTemp, Log, TEXT("[VoiceConversation] WebSocket connected."));
}

void UVoiceConversationSystem::OnWebSocketTranscription(const FString& TranscribedText)
{
	CurrentTranscribedText = TranscribedText;
	OnTranscriptionReceived.Broadcast(TranscribedText);

	UE_LOG(LogTemp, Log, TEXT("[VoiceConversation] WebSocket STT: %s"), *TranscribedText);
}

void UVoiceConversationSystem::OnWebSocketAudioStart()
{
	OnTTSStarted.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[VoiceConversation] WebSocket TTS streaming started."));
}
