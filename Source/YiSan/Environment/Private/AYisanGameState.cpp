// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AYisanGameState.h"

#include "ADasanActor.h"
#include "APlayerControl.h"
#include "GameLogging.h"
#include "Macro.h"
#include "AQuestManagerActor.h"
#include "EBuildingType.h"
#include "Net/UnrealNetwork.h"

AYisanGameState::AYisanGameState()
{
}

void AYisanGameState::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() && QuestManager)
	{
		QuestManager->StartTour();
	}
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

	if (APlayerControl* PC = Cast<APlayerControl>(GetWorld()->GetFirstPlayerController()))
	{
		PC->ClientRPC_ShowToastMessage(Message);
	}
}

void AYisanGameState::StartGlobalTour()
{
	if (!HasAuthority())
	{
		return;
	}

	if (QuestManager)
	{
		QuestManager->StartTour();
	}
	else
	{
		PRINTLOG(TEXT("YisanGameState: QuestManager not assigned when starting tour"));
	}

	if (DasanNPC)
	{
		DasanNPC->StartTour();
	}
	else
	{
		PRINTLOG(TEXT("YisanGameState: DasanNPC not assigned when starting tour"));
	}
}

void AYisanGameState::OnRep_QuestManager()
{
	if (QuestManager)
	{
		QuestManager->StartTour();
	}
}

void AYisanGameState::SetQuestManager(AQuestManagerActor* InManager)
{
	if (!HasAuthority())
	{
		return;
	}

	QuestManager = InManager;
	OnRep_QuestManager();
}

EBuildingType AYisanGameState::GetCurrentQuestTarget() const
{
	return QuestManager ? QuestManager->GetCurrentQuestTarget() : EBuildingType::None;
}

int32 AYisanGameState::GetCurrentQuestIndex() const
{
	return QuestManager ? QuestManager->GetCurrentQuestIndex() : INDEX_NONE;
}

bool AYisanGameState::HasActiveQuest() const
{
	return QuestManager ? QuestManager->HasActiveQuest() : false;
}
