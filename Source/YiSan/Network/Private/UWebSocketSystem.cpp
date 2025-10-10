/**
 * @file UWebSocketSystem.cpp
 * @brief UWebSocketSystem 구현에 대한 Doxygen 주석을 제공합니다.
 */
#include "UWebSocketSystem.h"
#include "NetworkLog.h"
#include "ENetworkLogType.h"
#include "WebSocketsModule.h"
#include "Sound/SoundWaveProcedural.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void UWebSocketSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UWebSocketSystem::Deinitialize()
{
	if (WebSocket.IsValid())
	{
		WebSocket->OnConnected().RemoveAll(this);
		WebSocket->OnConnectionError().RemoveAll(this);
		WebSocket->OnClosed().RemoveAll(this);
		WebSocket->OnMessage().RemoveAll(this);
		WebSocket->OnBinaryMessage().RemoveAll(this);
        
		if(WebSocket->IsConnected())
		{
			WebSocket->Close();
		}
	}
    
	Super::Deinitialize();
}

void UWebSocketSystem::Connect(const FString& Url)
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
		return;

	
	LogNetwork((TEXT("Connecting to %s"), *Url));

	FWebSocketsModule& WebSocketModule = FModuleManager::LoadModuleChecked<FWebSocketsModule>(TEXT("WebSockets"));
	WebSocket = WebSocketModule.CreateWebSocket(Url, TEXT("ws"));

	WebSocket->OnConnected().AddUObject(this, &UWebSocketSystem::OnConnected_Native);
	WebSocket->OnConnectionError().AddUObject(this, &UWebSocketSystem::OnConnectionError_Native);
	WebSocket->OnClosed().AddUObject(this, &UWebSocketSystem::OnClosed_Native);
	WebSocket->OnMessage().AddUObject(this, &UWebSocketSystem::OnMessageReceived_Native);
	WebSocket->OnBinaryMessage().AddUObject(this, &UWebSocketSystem::OnBinaryMessageReceived_Native);

	WebSocket->Connect();
}

void UWebSocketSystem::Close()
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		WebSocket->Close();
	}
}

void UWebSocketSystem::SendMessage(const FString& Message)
{
	if (IsConnected())
	{
		LogNetwork( (TEXT("Sending Message"), Message));
		WebSocket->Send(Message);
	}
}

bool UWebSocketSystem::IsConnected() const
{
	return WebSocket.IsValid() && WebSocket->IsConnected();
}

void UWebSocketSystem::OnConnected_Native()
{
	LogNetwork( TEXT("Connection successful."));
	OnConnected.Broadcast();
}

void UWebSocketSystem::OnConnectionError_Native(const FString& Error)
{
	LogNetwork( (TEXT("Connection failed: %s"), *Error));
	OnConnectionError.Broadcast(Error);
}

void UWebSocketSystem::OnClosed_Native(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	LogNetwork( (TEXT("Connection closed. Code: %d, Reason: %s, Clean: %s"),
		StatusCode, *Reason));
    
	OnClosed.Broadcast(StatusCode, Reason, bWasClean);
}

void UWebSocketSystem::OnMessageReceived_Native(const FString& Message)
{
	LogNetwork( (TEXT("OnMessageReceived : %s"), *Message));
	OnMessageReceived.Broadcast(Message);
}

void UWebSocketSystem::OnBinaryMessageReceived_Native(const void* Data, SIZE_T Size, bool bIsLastFragment)
{
	TArray<uint8> BinaryMessage;
	BinaryMessage.SetNumUninitialized(Size);
	FMemory::Memcpy(BinaryMessage.GetData(), Data, Size);

	LogNetwork( (TEXT("OnBinaryMessageReceived : Received %d bytes, Last Fragment: %s"), Size, bIsLastFragment ? TEXT("True") : TEXT("False")));

	// Check if it's a WAV file (simple header check)
	if (BinaryMessage.Num() >= 12 &&
		BinaryMessage[0] == 'R' && BinaryMessage[1] == 'I' && BinaryMessage[2] == 'F' && BinaryMessage[3] == 'F' &&
		BinaryMessage[8] == 'W' && BinaryMessage[9] == 'A' && BinaryMessage[10] == 'V' && BinaryMessage[11] == 'E')
	{
		// It's likely a WAV file, attempt to play it
		USoundWaveProcedural* SoundWave = NewObject<USoundWaveProcedural>();
		if (SoundWave)
		{
			SoundWave->SetSampleRate(16000); // Assuming 16kHz, adjust if necessary
			SoundWave->NumChannels = 1;      // Assuming mono, adjust if necessary
			SoundWave->Duration = INDEFINITELY_LOOPING_DURATION; // Or calculate from WAV header
			SoundWave->SoundGroup = ESoundGroup::SOUNDGROUP_Voice;
			SoundWave->bLooping = false;

			// Queue the audio data
			SoundWave->QueueAudio(BinaryMessage.GetData(), BinaryMessage.Num());

			// Play the sound
			if (UGameInstance* GameInstance = GetGameInstance())
			{
				if (UWorld* World = GameInstance->GetWorld())
				{
					UGameplayStatics::PlaySound2D(World, SoundWave);
					LogNetwork(TEXT("Playing received WAV audio."));
				}
				else
				{
					LogNetwork(TEXT("Failed to get World from GameInstance to play audio."));
				}
			}
			else
			{
				LogNetwork(TEXT("Failed to get GameInstance to play audio."));
			}
		}
		else
		{
			LogNetwork(TEXT("Failed to create USoundWaveProcedural."));
		}
	}
	else
	{
		// Not a WAV file, or header check failed, just broadcast the raw binary message
		OnBinaryMessageReceived.Broadcast(BinaryMessage, Size, bIsLastFragment);
		LogNetwork(TEXT("Received non-WAV binary message."));
	}
}

void UWebSocketSystem::LogNetwork( const FString& Message )
{
	NETWORK_LOG(TEXT("[WS]%s"), *Message);
}
