// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via
// any medium is strictly prohibited. Proprietary and confidential.

#include "AQuestManagerActor.h"

#include "AYisanGameState.h"
#include "GameLogging.h"
#include "Macro.h"
#include "UBroadcastManager.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AQuestManagerActor::AQuestManagerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);
}

void AQuestManagerActor::BeginPlay()
{
	Super::BeginPlay();

	EnsureBroadcastManager();

	if (HasAuthority())
	{
		if (BroadcastManager && !BroadcastManager->OnContactBuilding.IsAlreadyBound(this, &AQuestManagerActor::HandleBroadcastContact))
		{
			BroadcastManager->OnContactBuilding.AddDynamic(this, &AQuestManagerActor::HandleBroadcastContact);
		}

		InitializeQuestSteps();
		BroadcastCurrentQuest();
	}
	else if (HasActiveQuest())
	{
		BroadcastCurrentQuest();
	}
}

void AQuestManagerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AQuestManagerActor, CurrentQuestTarget);
	DOREPLIFETIME(AQuestManagerActor, CurrentQuestIndex);
}

void AQuestManagerActor::StartTour()
{
	if (HasAuthority())
	{
		InitializeQuestSteps();
	}

	BroadcastCurrentQuest();
}

void AQuestManagerActor::ServerRPC_NotifyContact_Implementation(EBuildingType InBuildingType)
{
	HandleContactInternal(InBuildingType);
}

void AQuestManagerActor::OnRep_CurrentQuestTarget()
{
	BroadcastCurrentQuest();
}

void AQuestManagerActor::OnRep_CurrentQuestIndex()
{
	// 현재 구현에서는 별도 처리가 필요하지 않지만, 확장 시 훅으로 사용됩니다.
}

void AQuestManagerActor::InitializeQuestSteps()
{
	if (bQuestInitialized)
	{
		return;
	}

	QuestSteps = {
		EBuildingType::Sinpungnu,
		EBuildingType::Uhwagwan,
		EBuildingType::Jwaikmun,
		EBuildingType::Bongsudang,
		EBuildingType::Yeomingak,
		EBuildingType::Byeolju,
	};

	const int32 InitialIndex = QuestSteps.Num() > 0 ? 0 : INDEX_NONE;
	const EBuildingType InitialTarget = QuestSteps.IsValidIndex(InitialIndex) ? QuestSteps[InitialIndex] : EBuildingType::None;
	ApplyQuestState(InitialTarget, InitialIndex);

	bQuestInitialized = true;
}

void AQuestManagerActor::EnsureBroadcastManager()
{
	if (!BroadcastManager)
	{
		BroadcastManager = UBroadcastManager::Get(GetWorld());
	}
}

void AQuestManagerActor::BroadcastCurrentQuest()
{
	EnsureBroadcastManager();

	if (BroadcastManager)
	{
		BroadcastManager->SendUpdateQuest(CurrentQuestTarget);
	}
}

void AQuestManagerActor::AdvanceQuest()
{
	if (!bQuestInitialized)
	{
		InitializeQuestSteps();
	}

	const int32 NextIndex = CurrentQuestIndex + 1;
	const EBuildingType NextTarget = QuestSteps.IsValidIndex(NextIndex) ? QuestSteps[NextIndex] : EBuildingType::None;

	ApplyQuestState(NextTarget, QuestSteps.IsValidIndex(NextIndex) ? NextIndex : INDEX_NONE);
	BroadcastCurrentQuest();
}

void AQuestManagerActor::ApplyQuestState(EBuildingType InTarget, int32 InQuestIndex)
{
	CurrentQuestTarget = InTarget;
	CurrentQuestIndex = InQuestIndex;
}

void AQuestManagerActor::HandleContactInternal(EBuildingType InBuildingType)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bQuestInitialized)
	{
		InitializeQuestSteps();
	}

	if (CurrentQuestTarget == EBuildingType::None)
	{
		PRINTLOG(TEXT("[QuestManager] HandleContactInternal ignored: quest already complete"));
		return;
	}

	if (InBuildingType != CurrentQuestTarget)
	{
		PRINTLOG(TEXT("[QuestManager] HandleContactInternal ignored: expected %s but received %s"),
			*ENUM_TO_NAME(EBuildingType, CurrentQuestTarget),
			*ENUM_TO_NAME(EBuildingType, InBuildingType));
		return;
	}

	PRINTLOG(TEXT("[QuestManager] Quest completed at %s"), *ENUM_TO_NAME(EBuildingType, InBuildingType));

	AdvanceQuest();
}

void AQuestManagerActor::HandleBroadcastContact(EBuildingType InBuildingType)
{
	HandleContactInternal(InBuildingType);
}

AQuestManagerActor* AQuestManagerActor::Get(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}

	if (AYisanGameState* GameState = Cast<AYisanGameState>(UGameplayStatics::GetGameState(WorldContextObject)))
	{
		return GameState->GetQuestManager();
	}

	return nullptr;
}

