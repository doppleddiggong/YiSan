// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AYisanGameState.h"
#include "ADasanActor.h"
#include "APlayerControl.h"
#include "GameLogging.h"
#include "AQuestManagerActor.h"
#include "EBuildingType.h"
#include "UGameSoundManager.h"
#include "UYisanOnlineSystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

// Static variable definition
int32 AYisanGameState::NextPlayerIndex = 0;

AYisanGameState::AYisanGameState()
{
}

void AYisanGameState::BeginPlay()
{
	Super::BeginPlay();

	// Register this GameState with NetworkSubsystem
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UYisanOnlineSystem* NetworkSubsystem = GI->GetSubsystem<UYisanOnlineSystem>())
		{
			NetworkSubsystem->SetGameState(this);
		}
	}
}

void AYisanGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYisanGameState, DasanNPC);
	DOREPLIFETIME(AYisanGameState, QuestManager);
	DOREPLIFETIME(AYisanGameState, PlayerList);
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

void AYisanGameState::ServerRPC_PlaySound_Implementation(EGameSoundType SoundType)
{
	if (HasAuthority())
	{
		MulticastRPC_PlaySound(SoundType);
	}
}

void AYisanGameState::MulticastRPC_PlaySound_Implementation(EGameSoundType SoundType)
{
	if (UGameSoundManager* SoundManager = UGameSoundManager::Get(GetWorld()))
	{
		SoundManager->PlaySound2D(SoundType);
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

void AYisanGameState::UpdatePlayerList()
{
	if (!HasAuthority())
		return;

	// Sort PlayerArray by PlayerIndex
	PlayerArray.Sort([](const APlayerState& A, const APlayerState& B)
	{
		const AYiSanPlayerState* PSA = Cast<const AYiSanPlayerState>(&A);
		const AYiSanPlayerState* PSB = Cast<const AYiSanPlayerState>(&B);

		if (!PSA || !PSB)
		{
			return A.GetPlayerName() < B.GetPlayerName();
		}

		return PSA->PlayerIndex < PSB->PlayerIndex;
	});

	TArray<FString> PlayerInfoList;

	for (int32 i = 0; i < PlayerArray.Num(); ++i)
	{
		if (auto PS = Cast<AYiSanPlayerState>(PlayerArray[i]))
		{
			PlayerInfoList.Add(FString::Printf(TEXT("%d:%s"), PS->PlayerIndex, *PS->Nickname));
		}
	}

	PlayerList = PlayerInfoList;
	PRINTLOG(TEXT("AYisanGameState: PlayerList updated. Current players: %s"),
		*FString::Join(PlayerList, TEXT(", ")));

	OnRep_PlayerList();
}

void AYisanGameState::OnRep_PlayerList()
{
	// Notify NetworkSubsystem on clients
	if (!HasAuthority())
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UYisanOnlineSystem* NetworkSubsystem = GI->GetSubsystem<UYisanOnlineSystem>())
			{
				NetworkSubsystem->OnPlayerListUpdated.Broadcast(PlayerList);
			}
		}
	}

	// Broadcast to direct delegates
	OnPlayerListUpdated.Broadcast(PlayerList);
}

void AYisanGameState::RequestRefreshPlayerList()
{
	if (HasAuthority())
	{
		UpdatePlayerList();
	}
	else
	{
		ServerRPC_UpdatePlayerList();
	}
}

void AYisanGameState::ServerRPC_UpdatePlayerList_Implementation()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AYisanGameState::UpdatePlayerList, 0.2f, false);
}