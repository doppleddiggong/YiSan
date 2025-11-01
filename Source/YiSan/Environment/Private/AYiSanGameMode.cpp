// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AYiSanGameMode.h"
#include "AYisanGameState.h"
#include "ADasanActor.h"
#include "AQuestManagerActor.h"
#include "FComponentHelper.h"
#include "GameLogging.h"

#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

static int32 NextPlayerIndex = 0;

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

void AYiSanGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	PRINTLOG( TEXT("[GameMode] PostLogin - PC=%s Pawn=%s"),
		*GetNameSafe(NewPlayer),
		*GetNameSafe(NewPlayer->GetPawn()));

	if (AYiSanPlayerState* PS = NewPlayer->GetPlayerState<AYiSanPlayerState>())
	{
		// 누적 인덱스 방식 (안 흔들림)
		PS->PlayerIndex = NextPlayerIndex++;
	}

	if (NewPlayer && !NewPlayer->GetPawn())
	{
		AActor* PlayerStart = FindPlayerStart(NewPlayer);
		APawn* NewPawn = SpawnDefaultPawnFor(NewPlayer, PlayerStart);

		if (NewPawn)
		{
			NewPlayer->Possess(NewPawn);
			PRINTLOG( TEXT("[GameMode] Forced possess: %s"), *GetNameSafe(NewPawn));
		}
		else
		{
			PRINTLOG( TEXT("[GameMode] SpawnDefaultPawnFor failed!"));
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

void AYiSanGameMode::StartTour()
{
	if (!HasAuthority())
		return;

	if (auto State = GetGameState<AYisanGameState>() )
	{
		State->StartGlobalTour();
		PRINTLOG( TEXT("[GameMode] Tour started via State"));
	}
	else
	{
		PRINTLOG( TEXT("[GameMode] State not found!"));
	}
}