// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UWebSocketSystem.h"
#include "NetworkData.h"
#include "WebSocketsModule.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "NetworkLog.h"

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

void UWebSocketSystem::RequestTTS(const FString& Text, const FString& ReferenceIndex, bool bUseCache)
{
	if (!IsConnected())
	{
		LogNetwork(TEXT("Cannot send TTS request. Not connected."));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("type"), TEXT("tts"));
	JsonObject->SetStringField(TEXT("text"), Text);
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
	LogNetwork(FString::Printf(TEXT("Sending binary audio data (%d bytes)"), AudioData.Num()));
	WebSocket->Send(AudioData.GetData(), AudioData.Num(), true);
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

void UWebSocketSystem::OnMessage_Native(const FString& Message)
{
	LogNetwork(FString::Printf(TEXT("Received message: %s"), *Message));

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		LogNetwork(FString::Printf(TEXT("Failed to parse incoming JSON message: %s"), *Message));
		return;
	}

	FString MessageType;
	if (!JsonObject->TryGetStringField(TEXT("type"), MessageType))
	{
		LogNetwork(FString::Printf(TEXT("Incoming JSON has no 'type' field: %s"), *Message));
		return;
	}

	if (MessageType == TEXT("transcription"))
	{
		FString TranscribedText;
		if (JsonObject->TryGetStringField(TEXT("text"), TranscribedText))
		{
			OnTranscriptionReceived.Broadcast(TranscribedText);
		}
	}
	else if (MessageType == TEXT("audio_start"))
	{
		bIsExpectingAudio = true;
		OnAudioStart.Broadcast();
	}
	else if (MessageType == TEXT("audio_end"))
	{
		bIsExpectingAudio = false;
		OnAudioEnd.Broadcast();
	}
	else if (MessageType == TEXT("completed"))
	{
		OnCompleted.Broadcast();
	}
	else if (MessageType == TEXT("pong"))
	{
		LogNetwork(TEXT("Received Pong"));
		// Optionally, broadcast a pong delegate
	}
	else
	{
		LogNetwork(FString::Printf(TEXT("Unknown message type received: %s"), *MessageType));
	}
}

void UWebSocketSystem::OnBinaryMessage_Native(const void* Data, SIZE_T Size, bool bIsLastFragment)
{
	LogNetwork(FString::Printf(TEXT("Received binary message (%d bytes)"), Size));
    
	TArray<uint8> DataArray;
	DataArray.Append(static_cast<const uint8*>(Data), Size);
    
	OnAudioChunkReceived.Broadcast(DataArray);
}

// --- Logging ---

void UWebSocketSystem::LogNetwork(const FString& Message)
{
	NETWORK_LOG(TEXT("[WS] %s"), *Message);
}