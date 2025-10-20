// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UMultiplayerSystem.h"
#include "UYiSanGameInstance.h"
#include "GameLogging.h"
#include "GameFramework/PlayerController.h"

UMultiplayerSystem::UMultiplayerSystem()
{
	// 이 컴포넌트는 Tick이 필요 없음
	PrimaryComponentTick.bCanEverTick = false;
}

void UMultiplayerSystem::BeginPlay()
{
	Super::BeginPlay();

	PRINTLOG(TEXT("[Multiplayer] BeginPlay - Owner=%s"), *GetNameSafe(GetOwner()));
}

// ========================================
// Session Management
// ========================================

bool UMultiplayerSystem::HostSession(const FString& MapName, int32 MaxPlayers)
{
	PRINTLOG(TEXT("[Multiplayer] HostSession - Map=%s, MaxPlayers=%d"), *MapName, MaxPlayers);

	UYiSanGameInstance* GameInstance = GetYiSanGameInstance();
	if (!GameInstance)
	{
		PRINTLOG(TEXT("[Multiplayer] HostSession failed - GameInstance is null"));
		return false;
	}

	bool bSuccess = GameInstance->HostGame(MapName, MaxPlayers);
	if (bSuccess)
	{
		bIsHost = true;
		bIsInSession = true;
	}

	return bSuccess;
}

bool UMultiplayerSystem::JoinSession(const FString& Address, int32 Port)
{
	PRINTLOG(TEXT("[Multiplayer] JoinSession - Address=%s, Port=%d"), *Address, Port);

	UYiSanGameInstance* GameInstance = GetYiSanGameInstance();
	if (!GameInstance)
	{
		PRINTLOG(TEXT("[Multiplayer] JoinSession failed - GameInstance is null"));
		return false;
	}

	bool bSuccess = GameInstance->JoinGame(Address, Port);
	if (bSuccess)
	{
		bIsHost = false;
		bIsInSession = true;
	}

	return bSuccess;
}

void UMultiplayerSystem::DisconnectSession()
{
	PRINTLOG(TEXT("[Multiplayer] DisconnectSession"));

	UYiSanGameInstance* GameInstance = GetYiSanGameInstance();
	if (!GameInstance)
	{
		PRINTLOG(TEXT("[Multiplayer] DisconnectSession failed - GameInstance is null"));
		return;
	}

	GameInstance->DisconnectFromSession();

	// 상태 초기화
	bIsHost = false;
	bIsInSession = false;
}

bool UMultiplayerSystem::IsHost() const
{
	// GameInstance의 상태를 우선 참조
	if (UYiSanGameInstance* GameInstance = GetYiSanGameInstance())
	{
		return GameInstance->IsHost();
	}

	// Fallback: 로컬 캐시 사용
	return bIsHost;
}

bool UMultiplayerSystem::IsInSession() const
{
	// GameInstance의 상태를 우선 참조
	if (UYiSanGameInstance* GameInstance = GetYiSanGameInstance())
	{
		return GameInstance->IsInSession();
	}

	// Fallback: 로컬 캐시 사용
	return bIsInSession;
}

// ========================================
// Internal Helper
// ========================================

UYiSanGameInstance* UMultiplayerSystem::GetYiSanGameInstance() const
{
	if (UWorld* World = GetWorld())
	{
		return Cast<UYiSanGameInstance>(World->GetGameInstance());
	}

	return nullptr;
}
