// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ALobbyGameMode.h"

#include "APlayerControl.h"
#include "AYiSanPlayerState.h"
#include "AYisanGameState.h"
#include "GameLogging.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"

ALobbyGameMode::ALobbyGameMode()
{
	// 난입 허용 설정
	PlayerControllerClass = APlayerControl::StaticClass();
	PlayerStateClass = AYiSanPlayerState::StaticClass();
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	AYisanGameState::NextPlayerIndex = 0;
	PRINTLOG(TEXT("[LobbyGameMode] BeginPlay - Reset NextPlayerIndex to 0"));
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Lobby is local-only, PlayerIndex will be assigned when joining host server
	PRINTLOG(TEXT("[LobbyGameMode] PostLogin - %s (PlayerIndex assignment skipped in lobby)"),
		*GetNameSafe(NewPlayer));

	// Pawn이 없으면 강제로 생성
	if (NewPlayer && !NewPlayer->GetPawn())
	{
		AActor* PlayerStart = FindPlayerStart(NewPlayer);
		APawn* NewPawn = SpawnDefaultPawnFor(NewPlayer, PlayerStart);
		if (NewPawn)
			NewPlayer->Possess(NewPawn);
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// Update player list after logout via GameState
	if (AYisanGameState* GS = GetWorld()->GetGameState<AYisanGameState>())
	{
		GS->UpdatePlayerList();
	}
}