// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AYisanGameState.h"
#include "ADasanActor.h"
#include "APlayerControl.h"
#include "GameLogging.h"
#include "UQuestManager.h"
#include "Net/UnrealNetwork.h"

AYisanGameState::AYisanGameState()
{
	GlobalTourState = EDasanState::Tour;
	bIsTourActive = false;
}

void AYisanGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYisanGameState, DasanNPC);
	DOREPLIFETIME(AYisanGameState, GlobalTourState);
	DOREPLIFETIME(AYisanGameState, bIsTourActive);
}

void AYisanGameState::ServerRPC_BroadcastToastMessage_Implementation(const FString& Message)
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


void AYisanGameState::StartGlobalTour()
{
	if (!HasAuthority())
		return;

	// QuestManager 초기화
	QuestManager = UQuestManager::Get(GetWorld());
	if (!QuestManager)
	{
		UE_LOG(LogTemp, Error, TEXT("YisanGameState: QuestManager not found!"));
		return;
	}

	if (QuestManager->GetCurTarget() == EBuildingType::None)
	{
		QuestManager->InitSystem();
	}

	if (DasanNPC)
	{
		bIsTourActive = true;
		GlobalTourState = EDasanState::Tour;

		DasanNPC->StartTour();
		UE_LOG(LogTemp, Log, TEXT("YisanGameState: Global tour started! First target: %d"),
			static_cast<uint8>(QuestManager->GetCurTarget()));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("YisanGameState: DasanNPC not found!"));
	}
}

void AYisanGameState::UpdateTourState(EDasanState NewState)
{
	if (!HasAuthority())
		return;

	GlobalTourState = NewState;

	UE_LOG(LogTemp, Log, TEXT("YisanGameState: Tour state updated - State: %d"),
		static_cast<uint8>(NewState));
}

int32 AYisanGameState::GetCurQuestIndex()
{
	if (!QuestManager)
		QuestManager = UQuestManager::Get(GetWorld());
	return QuestManager ? QuestManager->GetCurQuestIndex() : INDEX_NONE;
}

EBuildingType AYisanGameState::GetCurTargetBuilding()
{
	if (!QuestManager)
		QuestManager = UQuestManager::Get(GetWorld());
	return QuestManager ? QuestManager->GetCurTarget() : EBuildingType::None;
}