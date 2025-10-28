// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AYisanGameState.h"
#include "ADasanActor.h"
#include "APlayerActor.h"
#include "APlayerControl.h"
#include "GameLogging.h"
#include "UAnswerStateSystem.h"
#include "UQuestManager.h"
#include "EBuildingType.h"
#include "UBroadcastManager.h"
#include "Net/UnrealNetwork.h"

AYisanGameState::AYisanGameState()
{
}

void AYisanGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYisanGameState, DasanNPC);
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
		return;

	// QuestManager 초기화
	QuestManager = UQuestManager::Get(GetWorld());
	if (!QuestManager)
	{
		PRINTLOG(TEXT("YisanGameState: QuestManager not found!"));
		return;
	}

	if (QuestManager->GetCurTarget() == EBuildingType::None)
		QuestManager->InitSystem();

	DasanNPC->StartTour();
}
