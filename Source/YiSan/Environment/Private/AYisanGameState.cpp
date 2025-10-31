// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AYisanGameState.h"
#include "ADasanActor.h"
#include "APlayerControl.h"
#include "GameLogging.h"
#include "AQuestManagerActor.h"
#include "EBuildingType.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

AYisanGameState::AYisanGameState()
{
}

void AYisanGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYisanGameState, DasanNPC);
	DOREPLIFETIME(AYisanGameState, QuestManager);
}

void AYisanGameState::MulticastRPC_ToastMessage_Implementation(const FString& Message)
{
	// 각 클라이언트에서 실행됨                                                                                                                                         
	if (auto PC = Cast<APlayerControl>(GetWorld()->GetFirstPlayerController()))
	{
		PC->ClientRPC_ShowToastMessage(Message);
	}
}

void AYisanGameState::MulticastRPC_UpdateQuestTarget_Implementation(const EBuildingType InBuildingType)
{
	if (auto PC = Cast<APlayerControl>(GetWorld()->GetFirstPlayerController()))
	{
		PC->ClientRPC_UpdateQuestTarget(InBuildingType);
	}
}

void AYisanGameState::MulticastRPC_LoadingComplete_Implementation()
{
	if (!ensureMsgf(GetWorld(), TEXT("MulticastRPC_NotifyLoadingComplete requires a valid world")))
	{
		return;
	}

	for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (auto PC = Cast<APlayerControl>(It->Get()))
		{
			PC->HandleLoadingComplete();
		}
	}
}

void AYisanGameState::StartGlobalTour()
{
	if (!HasAuthority())
		return;

	// QuestManager 초기화
	if (!QuestManager)
	{
		QuestManager = AQuestManagerActor::Get(this);
		if (!QuestManager)
		{
			PRINTLOG(TEXT("YisanGameState: QuestManager actor not found!"));
			return;
		}
	}
	
	QuestManager->StartQuest();
	DasanNPC->StartTour();
}

void AYisanGameState::SetQuestManager(AQuestManagerActor* InQuestManager)
{
	if (!HasAuthority())
	{
		return;
	}

	QuestManager = InQuestManager;
	OnRep_QuestManager();
}

void AYisanGameState::OnRep_QuestManager()
{
	if (QuestManager)
	{
		QuestManager->SendUpdateQuest();
	}
}