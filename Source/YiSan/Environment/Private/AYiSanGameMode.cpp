// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AYiSanGameMode.h"
#include "AYisanGameState.h"
#include "ADasanActor.h"
#include "AQuestManagerActor.h"
#include "FComponentHelper.h"
#include "GameLogging.h"

#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

/**
 * @file AYiSanGameMode.cpp
 * @brief AYiSanGameMode의 동작을 구현합니다.
 */

/** @brief 새 게임 초기화 시 플레이어 인덱스 추적을 초기화합니다. */
void AYiSanGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	AYisanGameState::NextPlayerIndex = 0;
}

/** @brief 매치 시작 시 퀘스트 매니저가 준비되어 있도록 보장합니다. */
void AYiSanGameMode::BeginPlay()
{
	Super::BeginPlay();
	bUseSeamlessTravel = true;

	if (HasAuthority())
	{
		if (AYisanGameState* State = GetGameState<AYisanGameState>())
		{
			if (!State->GetQuestManager())
			{
				AQuestManagerActor* QuestManager = AQuestManagerActor::Get(this);
				if (!QuestManager)
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					QuestManager = GetWorld()->SpawnActor<AQuestManagerActor>(AQuestManagerActor::StaticClass(), SpawnParams);
				}

				State->SetQuestManager(QuestManager);
			}
		}
	}
}

/**
 * @brief 심리스 트래블 중 지속적인 플레이어 인덱스를 할당합니다.
 * @param C 새 맵으로 이동 중인 컨트롤러입니다.
 */
void AYiSanGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

	if (APlayerController* PC = Cast<APlayerController>(C))
	{
		if (AYiSanPlayerState* PS = Cast<AYiSanPlayerState>(PC->PlayerState))
		{
			if (PS->PlayerIndex < 0)
			{
				int32 AssignedIndex = AYisanGameState::NextPlayerIndex++;
				PS->SetPlayerIndex(AssignedIndex);
			}
		}
	}
}

/**
 * @brief 플레이어 인덱스를 부여하고 폰 존재를 확인하며 다산 NPC 참조를 캐시합니다.
 * @param NewPlayer 새로 합류한 플레이어 컨트롤러입니다.
 */
void AYiSanGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AYiSanPlayerState* PS = Cast<AYiSanPlayerState>(NewPlayer->PlayerState);
	if (!PS)
	{
		return;
	}

	if (PS->PlayerIndex < 0)
	{
		int32 AssignedIndex = AYisanGameState::NextPlayerIndex++;
		PS->SetPlayerIndex(AssignedIndex);
	}

	if (NewPlayer && !NewPlayer->GetPawn())
	{
		AActor* PlayerStart = FindPlayerStart(NewPlayer);
		APawn* NewPawn = SpawnDefaultPawnFor(NewPlayer, PlayerStart);

		if (NewPawn)
		{
			NewPlayer->Possess(NewPawn);
		}
	}

	// DasanNPC 찾기 및 GameState 설정
	if ( DasanNPC == nullptr )
	{
		auto FoundActors = FComponentHelper::GetAllOfClass<ADasanActor>(GetWorld());
		if (FoundActors.Num() > 0)
		{
			DasanNPC = FoundActors[0];
			// GameState에도 설정
			if (auto State = GetGameState<AYisanGameState>() )
			{
				State->DasanNPC = DasanNPC;
				PRINTLOG( TEXT("[GameMode] DasanNPC set to GameState"));
			}
		}
	}
}

/** @brief 모든 플레이어를 위한 공유 투어 시퀀스를 시작합니다. */
void AYiSanGameMode::StartTour()
{
	if (!HasAuthority())
		return;

	if (auto State = GetGameState<AYisanGameState>() )
	{
		State->StartGlobalTour();
	}
}