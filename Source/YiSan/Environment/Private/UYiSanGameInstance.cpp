// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UYiSanGameInstance.h"
#include "GameLogging.h"
#include "Macro.h"
#include "UBroadcastManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// ========================================
// Multiplayer Session Management
// ========================================

bool UYiSanGameInstance::HostGame(const FString& MapName, int32 MaxPlayers)
{
	if (!GetWorld())
	{
		PRINTLOG(TEXT("[GameInstance] HostGame failed - World is null"));
		return false;
	}

	PRINTLOG(TEXT("[GameInstance] HostGame - Map=%s, MaxPlayers=%d"), *MapName, MaxPlayers);

	// 서버로 레벨 전환 (?listen 옵션으로 리슨 서버 생성)
	FString TravelURL = FString::Printf(TEXT("/Game/CustomContents/Maps/%s?listen"), *MapName);

	// 옵션 추가: MaxPlayers
	TravelURL += FString::Printf(TEXT("?MaxPlayers=%d"), MaxPlayers);

	// ServerTravel 호출
	ServerTravel(TravelURL);

	// 상태 업데이트
	bIsHost = true;
	bIsInSession = true;

	// 브로드캐스트 이벤트 전송
	if (auto BroadcastManager = UBroadcastManager::Get(this))
	{
		BroadcastManager->SendSessionHost(MapName);
	}

	return true;
}

bool UYiSanGameInstance::JoinGame(const FString& Address, int32 Port)
{
	if (!GetWorld())
	{
		PRINTLOG(TEXT("[GameInstance] JoinGame failed - World is null"));
		return false;
	}

	if (Address.IsEmpty())
	{
		PRINTLOG(TEXT("[GameInstance] JoinGame failed - Address is empty"));
		return false;
	}

	PRINTLOG(TEXT("[GameInstance] JoinGame - Address=%s, Port=%d"), *Address, Port);

	// 클라이언트로 서버에 접속
	FString TravelURL = FString::Printf(TEXT("%s:%d"), *Address, Port);
	ClientTravel(TravelURL);

	// 상태 업데이트
	bIsHost = false;
	bIsInSession = true;

	// 브로드캐스트 이벤트 전송
	if (auto BroadcastManager = UBroadcastManager::Get(this))
	{
		BroadcastManager->SendSessionJoin(Address, Port);
	}

	return true;
}

void UYiSanGameInstance::DisconnectFromSession()
{
	if (!bIsInSession)
	{
		PRINTLOG(TEXT("[GameInstance] DisconnectFromSession - Not in session"));
		return;
	}

	PRINTLOG(TEXT("[GameInstance] DisconnectFromSession"));

	// 로비 맵으로 복귀
	UGameplayStatics::OpenLevel(this, TEXT("LobbyMap"), false);

	// 상태 초기화
	bIsHost = false;
	bIsInSession = false;

	// 브로드캐스트 이벤트 전송
	if (auto BroadcastManager = UBroadcastManager::Get(this))
	{
		BroadcastManager->SendSessionDisconnect();
	}
}

bool UYiSanGameInstance::IsHost() const
{
	return bIsHost;
}

bool UYiSanGameInstance::IsInSession() const
{
	return bIsInSession;
}

// ========================================
// Internal Travel Functions
// ========================================

void UYiSanGameInstance::ServerTravel(const FString& MapName)
{
	if (UWorld* World = GetWorld())
	{
		PRINTLOG(TEXT("[GameInstance] ServerTravel - URL=%s"), *MapName);
		World->ServerTravel(MapName);
	}
}

void UYiSanGameInstance::ClientTravel(const FString& Address)
{
	if (APlayerController* PC = GetFirstLocalPlayerController())
	{
		PRINTLOG(TEXT("[GameInstance] ClientTravel - Address=%s"), *Address);
		PC->ClientTravel(Address, TRAVEL_Absolute);
	}
}
