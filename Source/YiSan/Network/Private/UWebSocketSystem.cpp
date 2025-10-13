// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UWebSocketSystem.h"
#include "NetworkData.h"
#include "WebSocketsModule.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "NetworkLog.h"
#include "UBroadcastManger.h"
#include "Misc/Base64.h"

// --- Subsystem Lifecycle ---

void UWebSocketSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bIsExpectingAudio = false;
}

void UWebSocketSystem::Deinitialize()
{
	if (WebSocket.IsValid())
	{
		// Unbind all delegates
		WebSocket->OnConnected().RemoveAll(this);
		WebSocket->OnConnectionError().RemoveAll(this);
		WebSocket->OnClosed().RemoveAll(this);
		WebSocket->OnMessage().RemoveAll(this);
		WebSocket->OnBinaryMessage().RemoveAll(this);

		if (WebSocket->IsConnected())
		{
			WebSocket->Close();
		}
	}
	Super::Deinitialize();
}

// --- Client Actions ---

void UWebSocketSystem::Connect()
{
	if (IsConnected())
	{
		LogNetwork(TEXT("Already connected."));
		return;
	}

	const FString Url = NetworkConfig::GetSocketURL();
	if (Url.IsEmpty())
	{
		LogNetwork(TEXT("WebSocket URL is empty. Please check your configuration in DefaultMyNetwork.ini."));
		return;
	}

	LogNetwork(FString::Printf(TEXT("Connecting to %s"), *Url));

	FWebSocketsModule& WebSocketModule = FModuleManager::LoadModuleChecked<FWebSocketsModule>(TEXT("WebSockets"));
	WebSocket = WebSocketModule.CreateWebSocket(Url, TEXT("ws"));

	// Bind native delegates
	WebSocket->OnConnected().AddUObject(this, &UWebSocketSystem::OnConnected_Native);
	WebSocket->OnConnectionError().AddUObject(this, &UWebSocketSystem::OnConnectionError_Native);
	WebSocket->OnClosed().AddUObject(this, &UWebSocketSystem::OnClosed_Native);
	WebSocket->OnMessage().AddUObject(this, &UWebSocketSystem::OnMessage_Native);
	WebSocket->OnBinaryMessage().AddUObject(this, &UWebSocketSystem::OnBinaryMessage_Native);

	WebSocket->Connect();
}

void UWebSocketSystem::Disconnect()
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		LogNetwork(TEXT("Disconnecting..."));
		WebSocket->Close();
	}
}

bool UWebSocketSystem::IsConnected() const
{
	return WebSocket.IsValid() && WebSocket->IsConnected();
}

void UWebSocketSystem::RequestTTS(
	const FString& Text,
	const FString& VoiceName,
	float SpeakingRate,
	float Pitch,
	const FString& ReferenceIndex,
	bool bUseCache)
{
	if (!IsConnected())
	{
		LogNetwork(TEXT("Cannot send TTS request. Not connected."));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("type"), TEXT("tts"));
	JsonObject->SetStringField(TEXT("text"), Text);

	// TTS voice parameters
	if (!VoiceName.IsEmpty())
	{
		JsonObject->SetStringField(TEXT("voice_name"), VoiceName);
	}
	JsonObject->SetNumberField(TEXT("speaking_rate"), SpeakingRate);
	JsonObject->SetNumberField(TEXT("pitch"), Pitch);

	// Cache parameters
	JsonObject->SetStringField(TEXT("reference_index"), ReferenceIndex);
	JsonObject->SetBoolField(TEXT("use_cache"), bUseCache);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	LogNetwork(FString::Printf(TEXT("Sending TTS request: %s"), *OutputString));
	WebSocket->Send(OutputString);
}

void UWebSocketSystem::SendAudio(const TArray<uint8>& AudioData)
{
	if (!IsConnected())
	{
		LogNetwork(TEXT("Cannot send audio. Not connected."));
		return;
	}
	// LogNetwork(FString::Printf(TEXT("Sending binary audio data (%d bytes)"), AudioData.Num()));
	WebSocket->Send(AudioData.GetData(), AudioData.Num(), true);
}

void UWebSocketSystem::SetTTSConfig(const FString& VoiceName, float SpeakingRate, float Pitch)
{
	if (!IsConnected())
	{
		LogNetwork(TEXT("Cannot set TTS config. Not connected."));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("type"), TEXT("config"));

	// Only set fields that are not default values (indicating user wants to change them)
	if (!VoiceName.IsEmpty())
	{
		JsonObject->SetStringField(TEXT("voice_name"), VoiceName);
	}
	if (SpeakingRate >= 0.0f)
	{
		JsonObject->SetNumberField(TEXT("speaking_rate"), SpeakingRate);
	}
	if (Pitch != 999.0f)  // Use 999.0f as sentinel value to skip
	{
		JsonObject->SetNumberField(TEXT("pitch"), Pitch);
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	LogNetwork(FString::Printf(TEXT("Sending TTS config: %s"), *OutputString));
	WebSocket->Send(OutputString);
}

void UWebSocketSystem::SendPing()
{
	if (!IsConnected())
	{
		LogNetwork(TEXT("Cannot send ping. Not connected."));
		return;
	}
    
	LogNetwork(TEXT("Sending Ping"));
	WebSocket->Send(TEXT("{\"type\":\"ping\"}"));
}

void UWebSocketSystem::SendStartRecordingMessage()
{
	if (!IsConnected())
	{
		LogNetwork(TEXT("Cannot send start_recording message. Not connected."));
		return;
	}
    
	LogNetwork(TEXT("Sending start_recording message"));
	WebSocket->Send(TEXT("{\"type\":\"start_recording\"}"));
}

void UWebSocketSystem::SendStopRecordingMessage()
{
	if (!IsConnected())
	{
		LogNetwork(TEXT("Cannot send stop_recording message. Not connected."));
		return;
	}
    
	LogNetwork(TEXT("Sending stop_recording message"));
	WebSocket->Send(TEXT("{\"type\":\"stop_recording\"}"));
}


// --- Native WebSocket Callbacks ---

void UWebSocketSystem::OnConnected_Native()
{
	LogNetwork(TEXT("Connection successful."));
	bIsExpectingAudio = false;
	OnConnected.Broadcast();
}

void UWebSocketSystem::OnConnectionError_Native(const FString& Error)
{
	LogNetwork(FString::Printf(TEXT("Connection failed: %s"), *Error));
	OnConnectionError.Broadcast(Error);
}

void UWebSocketSystem::OnClosed_Native(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	LogNetwork(FString::Printf(TEXT("Connection closed. Code: %d, Reason: %s, Clean: %s"), StatusCode, *Reason, bWasClean ? TEXT("true") : TEXT("false")));
	bIsExpectingAudio = false;
	OnClosed.Broadcast(StatusCode, Reason, bWasClean);
}

void UWebSocketSystem::OnMessage_Native(const FString& InMessage)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InMessage);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		LogNetwork(FString::Printf(TEXT("Failed to parse incoming JSON message: %s"), *InMessage));
		return;
	}

	FString MessageType;
	if (!JsonObject->TryGetStringField(TEXT("type"), MessageType))
	{
		LogNetwork(FString::Printf(TEXT("Incoming JSON has no 'type' field: %s"), *InMessage));
		return;
	}

	if (MessageType == TEXT("transcription"))
	{
		LogNetwork(FString::Printf(TEXT("Received message: %s"), *InMessage));
		// 현재는 transcription을 따로 처리하지 않음
	}
	else if (MessageType == TEXT("agent_response"))
	{
		LogNetwork(FString::Printf(TEXT("Received message: %s"), *InMessage));
		
		FString GPTResponse;
		if (JsonObject->TryGetStringField(TEXT("text"), GPTResponse))
		{
			LogNetwork(FString::Printf(TEXT("GPT Response: %s"), *GPTResponse));
			OnAgentResponse.Broadcast(GPTResponse);

			if (auto EventManager = UBroadcastManger::Get(this))
				EventManager->SendToastMessage(GPTResponse);
		}
	}
	else if (MessageType == TEXT("audio_start"))
	{
		LogNetwork(FString::Printf(TEXT("Received message: %s"), *InMessage));
		
		bIsExpectingAudio = true;
		OnAudioStart.Broadcast();
	}
	else if (MessageType == TEXT("audio_data"))
	{
		// LogNetwork(FString::Printf(TEXT("Received message: %s"), *InMessage));
		
		FString Base64Data;
		if (JsonObject->TryGetStringField(TEXT("data"), Base64Data))
		{
			TArray<uint8> DecodedAudio;
			if (FBase64::Decode(Base64Data, DecodedAudio))
			{
				LogNetwork(FString::Printf(TEXT("Received audio_data (%d bytes decoded)"), DecodedAudio.Num()));
				OnAudioDataReceived.Broadcast(DecodedAudio);
			}
			else
			{
				LogNetwork(TEXT("Failed to decode Base64 audio data"));
			}
		}
	}
	else if (MessageType == TEXT("audio_end"))
	{
		LogNetwork(FString::Printf(TEXT("Received message: %s"), *InMessage));
		bIsExpectingAudio = false;
		// 오디오 종료는 로그만 남김
	}
	else if (MessageType == TEXT("completed"))
	{
		LogNetwork(FString::Printf(TEXT("Received message: %s"), *InMessage));
		// 완료 메시지는 로그만 남김
	}
	else if (MessageType == TEXT("pong"))
	{
		LogNetwork(TEXT("Received Pong"));
	}
	else if (MessageType == TEXT("config_ack"))
	{
		LogNetwork(FString::Printf(TEXT("Received message: %s"), *InMessage));
		// Config 확인 메시지는 로그만 남김
	}
	else if (MessageType == TEXT("ack_start_recording"))
	{
		LogNetwork(FString::Printf(TEXT("Received message: %s"), *InMessage));
		
		FString TempMessage;
		JsonObject->TryGetStringField(TEXT("message"), TempMessage);
		OnStartRecordingAck.Broadcast(TempMessage);
	}
	else if (MessageType == TEXT("error"))
	{
		LogNetwork(FString::Printf(TEXT("Received message: %s"), *InMessage));
		
		FString ErrorMessage;
		if (JsonObject->TryGetStringField(TEXT("message"), ErrorMessage))
		{
			LogNetwork(FString::Printf(TEXT("Server Error: %s"), *ErrorMessage));
			OnError.Broadcast(ErrorMessage);
		}
		else
		{
			LogNetwork(TEXT("Server Error: (no message field)"));
			OnError.Broadcast(TEXT("Unknown server error"));
		}
	}
	else
	{
		LogNetwork(FString::Printf(TEXT("Unknown message type received: %s"), *MessageType));
	}
}

void UWebSocketSystem::OnBinaryMessage_Native(const void* Data, SIZE_T Size, bool bIsLastFragment)
{
	LogNetwork(FString::Printf(TEXT("Received binary message (%d bytes) - Currently not supported"), Size));
	// 바이너리 메시지는 현재 사용하지 않음 (모든 오디오는 JSON의 Base64로 전송됨)
}

// --- Logging ---

void UWebSocketSystem::LogNetwork(const FString& Message)
{
	NETWORK_LOG(TEXT("[WS] %s"), *Message);
}