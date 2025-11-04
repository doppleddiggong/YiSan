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

/**
 * @file AYisanGameState.cpp
 * @brief AYisanGameState의 동작을 구현합니다.
 */

// Static variable definition
int32 AYisanGameState::NextPlayerIndex = 0;

/** @brief 게임 상태의 기본 생성자입니다. */
AYisanGameState::AYisanGameState()
{
}

/** @brief 플레이 시작 시 온라인 서브시스템에 게임 상태를 등록합니다. */
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

/** @brief 엔진에 복제 상태 멤버를 선언합니다. */
void AYisanGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYisanGameState, DasanNPC);
	DOREPLIFETIME(AYisanGameState, QuestManager);
	DOREPLIFETIME(AYisanGameState, PlayerList);
}

/** @brief 연결된 모든 클라이언트에 토스트 메시지를 표시합니다. */
void AYisanGameState::MulticastRPC_ToastMessage_Implementation(const FString& Message)
{
	// 각 클라이언트에서 실행됨                                                                                                                                         
	if (auto PC = Cast<APlayerControl>(GetWorld()->GetFirstPlayerController()))
	{
		PC->ClientRPC_ShowToastMessage(Message);
	}
}

/** @brief 퀘스트 목표 업데이트를 모든 클라이언트에 전파합니다. */
void AYisanGameState::MulticastRPC_UpdateQuestTarget_Implementation(const EBuildingType InBuildingType)
{
	if (auto PC = Cast<APlayerControl>(GetWorld()->GetFirstPlayerController()))
	{
		PC->ClientRPC_UpdateQuestTarget(InBuildingType);
	}
}

/** @brief 레벨 로딩 시퀀스가 완료되었음을 컨트롤러에 알립니다. */
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

/** @brief 공유 오디오 큐 재생을 위한 서버 진입점입니다. */
void AYisanGameState::ServerRPC_PlaySound_Implementation(EGameSoundType SoundType)
{
	if (HasAuthority())
	{
		MulticastRPC_PlaySound(SoundType);
	}
}

/** @brief 각 클라이언트에서 복제된 사운드 큐를 재생합니다. */
void AYisanGameState::MulticastRPC_PlaySound_Implementation(EGameSoundType SoundType)
{
	if (UGameSoundManager* SoundManager = UGameSoundManager::Get(GetWorld()))
	{
		SoundManager->PlaySound2D(SoundType);
	}
}

/** @brief 공유 투어 시퀀스를 시작하고 퀘스트 진행을 전개합니다. */
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

/** @brief 권한 측에서 퀘스트 매니저 액터를 설정합니다. */
void AYisanGameState::SetQuestManager(AQuestManagerActor* InQuestManager)
{
	if (!HasAuthority())
	{
		return;
	}

	QuestManager = InQuestManager;
	OnRep_QuestManager();
}

/** @brief 퀘스트 매니저 참조가 변경되면 UI 리스너를 갱신합니다. */
void AYisanGameState::OnRep_QuestManager()
{
	if (QuestManager)
	{
		QuestManager->SendUpdateQuest();
	}
}

/** @brief 연결된 플레이어의 복제 목록을 재구성합니다. */
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

/** @brief 플레이어 목록이 클라이언트에서 갱신될 때 서브시스템과 델리게이트에 알립니다. */
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

/** @brief 필요 시 서버로 위임하여 플레이어 목록 새로고침을 요청합니다. */
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

/** @brief 경쟁 상태를 방지하기 위해 서버 권한에서 플레이어 목록 재구성을 지연시킵니다. */
void AYisanGameState::ServerRPC_UpdatePlayerList_Implementation()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AYisanGameState::UpdatePlayerList, 0.2f, false);
}