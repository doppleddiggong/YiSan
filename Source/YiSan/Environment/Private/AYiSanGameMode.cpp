// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AYiSanGameMode.h"
#include "AYisanGameState.h"
#include "ADasanActor.h"
#include "AQuestManagerActor.h"
#include "FComponentHelper.h"
#include "GameLogging.h"

#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

void AYiSanGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Reset PlayerIndex counter at the start of each level
	// HandleSeamlessTravelPlayer and PostLogin will assign indices in order
	AYisanGameState::NextPlayerIndex = 0;
	PRINTLOG(TEXT("[GameMode] InitGame - Reset NextPlayerIndex to 0 (Map: %s)"), *MapName);
}

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

void AYiSanGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

	// Assign PlayerIndex for seamless travel players (Host doesn't go through PostLogin)
	if (APlayerController* PC = Cast<APlayerController>(C))
	{
		if (AYiSanPlayerState* PS = Cast<AYiSanPlayerState>(PC->PlayerState))
		{
			PRINTLOG(TEXT("[GameMode] HandleSeamlessTravelPlayer - PC=%s, PlayerState=%s, PlayerIndex=%d (before), Nickname=%s"),
				*GetNameSafe(PC),
				*GetNameSafe(PS),
				PS->PlayerIndex,
				*PS->Nickname);

			// Assign PlayerIndex if not already set
			if (PS->PlayerIndex < 0)
			{
				int32 AssignedIndex = AYisanGameState::NextPlayerIndex++;
				PS->SetPlayerIndex(AssignedIndex);
				PRINTLOG(TEXT("[GameMode] HandleSeamlessTravelPlayer - Assigned PlayerIndex %d (NextPlayerIndex is now %d)"),
					AssignedIndex, AYisanGameState::NextPlayerIndex);
			}
			else
			{
				PRINTLOG(TEXT("[GameMode] HandleSeamlessTravelPlayer - PlayerIndex already set, skipping"));
			}
		}
		else
		{
			PRINTLOG(TEXT("[GameMode] HandleSeamlessTravelPlayer - PC=%s, PlayerState is null or cast failed!"),
				*GetNameSafe(PC));
		}
	}
}

void AYiSanGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	PRINTLOG( TEXT("[GameMode] PostLogin - PC=%s Pawn=%s PlayerState=%s"),
		*GetNameSafe(NewPlayer),
		*GetNameSafe(NewPlayer->GetPawn()),
		*GetNameSafe(NewPlayer->PlayerState));

	AYiSanPlayerState* PS = Cast<AYiSanPlayerState>(NewPlayer->PlayerState);
	if (!PS)
	{
		PRINTLOG(TEXT("[GameMode] ERROR: PlayerState is null or cast failed!"));
		return;
	}

	// Set PlayerIndex using GameState's static counter (only if not already set)
	if (PS->PlayerIndex < 0)
	{
		int32 AssignedIndex = AYisanGameState::NextPlayerIndex++;
		PS->SetPlayerIndex(AssignedIndex);
		PRINTLOG(TEXT("[GameMode] Assigned PlayerIndex %d to %s (NextPlayerIndex is now %d)"),
			AssignedIndex, *GetNameSafe(NewPlayer), AYisanGameState::NextPlayerIndex);
	}
	else
	{
		PRINTLOG(TEXT("[GameMode] PlayerIndex already set to %d for %s - skipping assignment"),
			PS->PlayerIndex, *GetNameSafe(NewPlayer));
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