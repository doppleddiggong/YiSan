// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ALobbyGameMode.h"

#include "APlayerControl.h"
#include "AYiSanPlayerState.h"
#include "GameLogging.h"
#include "Macro.h"
#include "UNetworkGameInstanceSubsystem.h"
#include "UYiSanGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "YiSanPlayerListManager.h"
#include "UNetworkGameInstanceSubsystem.h" // Added include

ALobbyGameMode::ALobbyGameMode()
{
	// 난입 허용 설정
	bAllowJoinInProgress = true;

	PlayerControllerClass = APlayerControl::StaticClass();
	PlayerStateClass = AYiSanPlayerState::StaticClass();
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	CurrentPlayerCount++;

	PRINTLOG(TEXT("[LobbyGameMode] PostLogin - PC=%s, PlayerCount=%d/%d"),
		*GetNameSafe(NewPlayer),
		CurrentPlayerCount,
		MaxPlayers);

	// Pawn이 없으면 강제로 생성
	if (NewPlayer && !NewPlayer->GetPawn())
	{
		AActor* PlayerStart = FindPlayerStart(NewPlayer);
		APawn* NewPawn = SpawnDefaultPawnFor(NewPlayer, PlayerStart);

		if (NewPawn)
		{
			NewPlayer->Possess(NewPawn);
			PRINTLOG(TEXT("[LobbyGameMode] Forced possess: %s"), *GetNameSafe(NewPawn));
		}
		else
		{
			PRINTLOG(TEXT("[LobbyGameMode] SpawnDefaultPawnFor failed!"));
		}
	}

	OnPlayerLoggedIn.Broadcast(NewPlayer);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UNetworkGameInstanceSubsystem* NetworkSubsystem = GameInstance->GetSubsystem<UNetworkGameInstanceSubsystem>())
		{
			NetworkSubsystem->RequestPlayerListRefresh();
		}
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	CurrentPlayerCount = FMath::Max(0, CurrentPlayerCount - 1);

	PRINTLOG(TEXT("[LobbyGameMode] Logout - PC=%s, PlayerCount=%d/%d"),
		*GetNameSafe(Exiting),
		CurrentPlayerCount,
		MaxPlayers);

	OnPlayerLoggedOut.Broadcast(Exiting);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UNetworkGameInstanceSubsystem* NetworkSubsystem = GameInstance->GetSubsystem<UNetworkGameInstanceSubsystem>())
		{
			NetworkSubsystem->RequestPlayerListRefresh();
		}
	}
}

int32 ALobbyGameMode::GetCurrentPlayerCount() const
{
	return CurrentPlayerCount;
}

bool ALobbyGameMode::IsLobbyFull() const
{
	return CurrentPlayerCount >= MaxPlayers;
}
