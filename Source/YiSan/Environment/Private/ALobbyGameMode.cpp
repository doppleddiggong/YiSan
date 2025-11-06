// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ALobbyGameMode.h"

#include "APlayerControl.h"
#include "AYiSanPlayerState.h"
#include "AYisanGameState.h"
#include "GameLogging.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"

/**
 * @file ALobbyGameMode.cpp
 * @brief ALobbyGameMode의 동작을 구현합니다.
 */

/** @brief 로비 플레이 경험을 위한 컨트롤러 및 플레이어 상태 클래스를 구성합니다. */
ALobbyGameMode::ALobbyGameMode()
{
	// 난입 허용 설정
	PlayerControllerClass = APlayerControl::StaticClass();
	PlayerStateClass = AYiSanPlayerState::StaticClass();
}

/** @brief 레벨 시작 시 로비 상태를 초기화합니다. */
void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	AYisanGameState::NextPlayerIndex = 0;
	PRINTLOG(TEXT("[LobbyGameMode] BeginPlay - Reset NextPlayerIndex to 0"));
}

/**
 * @brief 참가 플레이어용 폰을 스폰하고 도착을 로그합니다.
 * @param NewPlayer 새로 연결된 플레이어를 나타내는 컨트롤러입니다.
 */
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

/** @brief 컨트롤러가 로비를 떠날 때 플레이어 목록 데이터를 정리합니다. */
void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// Update player list after logout via GameState
	if (AYisanGameState* GS = GetWorld()->GetGameState<AYisanGameState>())
	{
		GS->UpdatePlayerList();
	}
}