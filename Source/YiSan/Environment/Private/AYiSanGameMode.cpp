// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AYiSanGameMode.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void AYiSanGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogTemp, Warning, TEXT("[YiSanGameMode] PostLogin - PC=%s Pawn=%s"),
		*GetNameSafe(NewPlayer), *GetNameSafe(NewPlayer->GetPawn()));

	if (NewPlayer && !NewPlayer->GetPawn())
	{
		AActor* PlayerStart = FindPlayerStart(NewPlayer);
		APawn* NewPawn = SpawnDefaultPawnFor(NewPlayer, PlayerStart);

		if (NewPawn)
		{
			NewPlayer->Possess(NewPawn);
			UE_LOG(LogTemp, Log, TEXT("[YiSanGameMode] Forced possess: %s"), *GetNameSafe(NewPawn));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[YiSanGameMode] SpawnDefaultPawnFor failed!"));
		}
	}
}
