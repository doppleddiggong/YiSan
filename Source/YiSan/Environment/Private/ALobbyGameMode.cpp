// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ALobbyGameMode.h"

#include "APlayerControl.h"
#include "AYiSanPlayerState.h"
#include "YiSanPlayerListManager.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ALobbyGameMode::ALobbyGameMode()
{
	// 난입 허용 설정
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

	// Manually update player list after logout
	if (auto PM = Cast<AYiSanPlayerListManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AYiSanPlayerListManager::StaticClass())))
	{
		PM->BroadcastPlayerList();
	}
}