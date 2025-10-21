// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AYiSanGameMode.h"
#include "AYisanGameState.h"
#include "ADasanActor.h"
#include "FComponentHelper.h"
#include "GameLogging.h"
#include "Macro.h"

#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

void AYiSanGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	PRINTLOG( TEXT("[GameMode] PostLogin - PC=%s Pawn=%s"),
		*GetNameSafe(NewPlayer),
		*GetNameSafe(NewPlayer->GetPawn()));

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

void AYiSanGameMode::SetTourState(EDasanState InState)
{
	if (!HasAuthority())
		return;

	if (DasanNPC)
	{
		DasanNPC->ServerRPC_SetDasanState( InState );
		PRINTLOG( TEXT("[GameMode] Tour state set to: %s"), *ENUM_TO_NAME(EDasanState, InState));
	}
}