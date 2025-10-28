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

// void AYisanGameState::ServerRPC_SetDasanState_Implementation(EDasanState InState)
// {
// 	if (!HasAuthority())
// 		return;
// 	
// 	DasanNPC->DasanState = InState;
// }
//
// void AYisanGameState::ServerRPC_ToastMessage_Implementation(const FString& Message)
// {
// 	if (!HasAuthority())
// 		return;
//
// 	if (Message.IsEmpty())
// 		return;
//
// 	UWorld* World = GetWorld();
// 	if (!World)
// 		return;
//
// 	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
// 	{
// 		APlayerController* PC = It->Get();
// 		if (!PC)
// 			continue;
//
// 		if (APlayerControl* CustomPC = Cast<APlayerControl>(PC))
// 			CustomPC->ClientRPC_ShowToastMessage(Message);
// 		else
// 			PC->ClientMessage(Message);
// 	}
// }
//
// void AYisanGameState::ServerRPC_TryStartAnswer_Implementation(const FString& PlayerName)
// {
// 	if (!HasAuthority())
// 		return;
// 	
// 	DasanNPC->AnswerStateSystem->TryStartAnswer(PlayerName);
// }
//
// void AYisanGameState::ServerRPC_AnswerReply_Implementation()
// {
// 	if (!HasAuthority())
// 		return;
// 	
// 	DasanNPC->AnswerStateSystem->AnswerReply();
// }
//
// void AYisanGameState::ServerRPC_FinishAnswer_Implementation()
// {
// 	if (!HasAuthority())
// 		return;
// 	
// 	DasanNPC->AnswerStateSystem->FinishAnswer();
// }



//
// void AYisanGameState::ServerRPC_RecordingStart_Implementation(APlayerController* Player)
// {
// 	if (!HasAuthority())
// 		return;
//
// 	if (!Player || !Player->GetPawn())
// 	{
// 		PRINTLOG(TEXT("[GameState] ServerRPC_RecordingStart - Invalid Player!"));
// 		return;
// 	}
//
// 	// 플레이어 이름 가져오기
// 	FString PlayerName = TEXT("Unknown");
// 	if (auto PlayerActor = Cast<APlayerActor>(Player->GetPawn()))
// 	{
// 		PlayerName = PlayerActor->GetPlayerDisplayName();
// 	}
//
// 	PRINTLOG(TEXT("[GameState] ServerRPC_RecordingStart - Player: %s"), *PlayerName);
//
// 	// Dasan 유효성 체크
// 	if (!DasanNPC)
// 	{
// 		PRINTLOG(TEXT("[GameState] ServerRPC_RecordingStart - DasanNPC is nullptr!"));
// 		return;
// 	}
//
// 	if (!DasanNPC->AnswerStateSystem)
// 	{
// 		PRINTLOG(TEXT("[GameState] ServerRPC_RecordingStart - AnswerStateSystem is nullptr!"));
// 		return;
// 	}
//
// 	// Dasan에게 Answer 시작 시도
// 	PRINTLOG(TEXT("[GameState] ServerRPC_RecordingStart - Calling TryStartAnswer"));
// 	DasanNPC->AnswerStateSystem->TryStartAnswer(PlayerName);
//
// 	// BroadcastManager를 통해 UI 업데이트 (선택사항)
// 	if (UBroadcastManager* BM = UBroadcastManager::Get(GetWorld()))
// 	{
// 		BM->SendAudioCapture(true);
// 	}
// }
//
// void AYisanGameState::ServerRPC_RecordingEnd_Implementation(APlayerController* Player)
// {
// 	if (!HasAuthority())
// 		return;
//
// 	PRINTLOG(TEXT("[GameState] ServerRPC_NotifyRecordingEnd - Player: %s"),
// 		Player ? *Player->GetName() : TEXT("nullptr"));
//
// 	if (UBroadcastManager* BM = UBroadcastManager::Get(GetWorld()))
// 	{
// 		BM->SendAudioCapture(false);
// 	}
// }

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
