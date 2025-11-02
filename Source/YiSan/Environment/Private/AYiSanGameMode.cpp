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

	AYisanGameState::NextPlayerIndex = 0;
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

void AYiSanGameMode::StartTour()
{
	if (!HasAuthority())
		return;

	if (auto State = GetGameState<AYisanGameState>() )
	{
		State->StartGlobalTour();
	}
}