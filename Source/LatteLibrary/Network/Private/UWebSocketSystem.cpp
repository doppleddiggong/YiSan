/**
 * @file UWebSocketSystem.cpp
 * @brief UWebSocketSystem 구현에 대한 Doxygen 주석을 제공합니다.
 */
#include "UWebSocketSystem.h"
#include "NetworkLog.h"
#include "ENetworkLogType.h"
#include "WebSocketsModule.h"

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
        
		if(WebSocket->IsConnected())
		{
			WebSocket->Close();
		}
	}
    
	Super::Deinitialize();
}

void UWebSocketSystem::Connect(const FString& Url, const FString& Protocol)
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
		return;

	
	LogNetwork((TEXT("Connecting to %s"), *Url));

	FWebSocketsModule& WebSocketModule = FModuleManager::LoadModuleChecked<FWebSocketsModule>(TEXT("WebSockets"));
	WebSocket = WebSocketModule.CreateWebSocket(Url, Protocol);

	WebSocket->OnConnected().AddUObject(this, &UWebSocketSystem::OnConnected_Native);
	WebSocket->OnConnectionError().AddUObject(this, &UWebSocketSystem::OnConnectionError_Native);
	WebSocket->OnClosed().AddUObject(this, &UWebSocketSystem::OnClosed_Native);
	WebSocket->OnMessage().AddUObject(this, &UWebSocketSystem::OnMessageReceived_Native);

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

void UWebSocketSystem::LogNetwork( const FString& Message )
{
	NETWORK_LOG(TEXT("[WS]%s"), *Message);
}
