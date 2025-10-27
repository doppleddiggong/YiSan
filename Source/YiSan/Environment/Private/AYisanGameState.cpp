// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AYisanGameState.h"
#include "ADasanActor.h"
#include "APlayerControl.h"
#include "GameLogging.h"
#include "UAnswerStateSystem.h"
#include "UQuestManager.h"
#include "EBuildingType.h"
#include "Net/UnrealNetwork.h"

AYisanGameState::AYisanGameState()
{
}

void AYisanGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYisanGameState, DasanNPC);
}

void AYisanGameState::ServerRPC_ToastMessage_Implementation(const FString& Message)
{
	if (!HasAuthority())
		return;

	if (Message.IsEmpty())
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
			continue;

		if (APlayerControl* CustomPC = Cast<APlayerControl>(PC))
			CustomPC->ClientRPC_ShowToastMessage(Message);
		else
			PC->ClientMessage(Message);
	}
}

void AYisanGameState::ServerRPC_TryStartAnswer_Implementation(const FString& PlayerName)
{
	DasanNPC->AnswerStateSystem->TryStartAnswer(PlayerName);
}

void AYisanGameState::ServerRPC_FinishAnswer_Implementation()
{
	DasanNPC->AnswerStateSystem->FinishAnswer();
}

void AYisanGameState::StartGlobalTour()
{
	if (!HasAuthority())
		return;

	// QuestManager 초기화
	QuestManager = UQuestManager::Get(GetWorld());
	if (!QuestManager)
	{
		PRINTLOG(TEXT("YisanGameState: QuestManager not found!"));
		return;
	}

	if (QuestManager->GetCurTarget() == EBuildingType::None)
	{
		QuestManager->InitSystem();
	}

	if (DasanNPC)
	{
		DasanNPC->StartTour();
		PRINTLOG( TEXT("YisanGameState: Global tour started! First target: %s"), *ENUM_TO_NAME(EBuildingType, QuestManager->GetCurTarget())) ;
	}
	else
	{
		PRINTLOG( TEXT("YisanGameState: DasanNPC not found!"));
	}
}
