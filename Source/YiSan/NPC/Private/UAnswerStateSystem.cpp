// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UAnswerStateSystem.h"
#include "ADasanActor.h"
#include "APlayerActor.h"
#include "GameLogging.h"
#include "UBroadcastManager.h"
#include "UDialogManager.h"
#include "AIController.h"
#include "Macro.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

UAnswerStateSystem::UAnswerStateSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAnswerStateSystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAnswerStateSystem, CurrentQuestionerName);
}

void UAnswerStateSystem::InitSystem(ADasanActor* InOwner)
{
	OwnerDasan = InOwner;
	PrevState = EAnswerState::AnswerListen;
	PreviousMainState = EDasanState::Tour;

	// BroadcastManager 가져오기 및 이벤트 구독
	BroadcastManager = UBroadcastManager::Get(GetWorld());
	if (BroadcastManager)
	{
		BroadcastManager->OnAudioCapture.AddDynamic(this, &UAnswerStateSystem::OnAudioCaptureChanged);
		BroadcastManager->OnTTSPlaybackFinished.AddDynamic(this, &UAnswerStateSystem::OnTTSPlaybackFinished);
		PRINTLOG(TEXT("[AnswerSystem] BroadcastManager 이벤트 구독 성공 (OnAudioCapture, OnTTSPlaybackFinished)"));
	}
	else
	{
		PRINTLOG(TEXT("[AnswerSystem] BroadcastManager를 찾을 수 없습니다!"));
	}
}

void UAnswerStateSystem::SetAnswerState(const EAnswerState InState)
{
	CurState = InState;

	// 상태가 변경되었으므로 위젯 업데이트
	if (OwnerDasan)
	{
		OwnerDasan->UpdateWidgetState();
	}
}

bool UAnswerStateSystem::IsUpdateEnble()
{
	if ( OwnerDasan == nullptr)
		return false;

	if ( OwnerDasan->HasAuthority() == false)
		return false;
	return true;
}

void UAnswerStateSystem::UpdateTick(float DeltaTime )
{
	if ( !IsUpdateEnble())
		return;

	if (PrevState != CurState)
	{
		switch (CurState)
		{
			case EAnswerState::AnswerListen: Enter_AnswerListen();	break;
			case EAnswerState::AnswerReply:	 Enter_AnswerReply();	break;
			case EAnswerState::AnswerEnd:	 Enter_AnswerEnd();		break;
			default: break;
		}
		PrevState = CurState;
	}

	switch (CurState)
	{
		case EAnswerState::AnswerListen: Tick_AnswerListen(DeltaTime); break;
		case EAnswerState::AnswerReply:	Tick_AnswerReply(DeltaTime); break;
		default: break;
	}
}

// Enter 함수들
void UAnswerStateSystem::Enter_AnswerListen()
{
	PRINTLOG(TEXT("[AnswerState] Enter AnswerListen - Waiting for questions..."));
	PRINTLOG(TEXT("[AnswerState] 다산이 %s님의 질문을 듣고 있습니다."),
		CurrentQuestionerName.IsEmpty() ? TEXT("누군가") : *CurrentQuestionerName);

	ListenTimer = 0.0f;

	// UI 위젯 표시 (다산이 듣고 있다는 표시)
	if (BroadcastManager)
	{
		BroadcastManager->SendDasanListening(true, CurrentQuestionerName);
		PRINTLOG(TEXT("[AnswerState] SendDasanListening(true, %s)"), *CurrentQuestionerName);
	}
}

void UAnswerStateSystem::Enter_AnswerReply()
{
	PRINTLOG( TEXT("[AnswerState] Enter AnswerReply - Answering question..."));
	ReplyTimer = 0.0f;

	// 여기서 블루프린트 이벤트 호출 가능
	// 예: 답변 음성 재생, 자막 표시 등
}

void UAnswerStateSystem::Enter_AnswerEnd()
{
	PRINTLOG( TEXT("[AnswerState] Answer session ended"));

	// 다음 퀘스트로 이동
	if (OwnerDasan)
		OwnerDasan->NextQuest();
}

// Tick 함수들
void UAnswerStateSystem::Tick_AnswerListen(float DeltaTime)
{
	if (!OwnerDasan)
		return;

	ListenTimer += DeltaTime;

	// 타임아웃 시 자동으로 다음 웨이포인트로
	if (ListenTimer >= ListenTimeout)
	{
		PRINTLOG( TEXT("[AnswerState] Listen timeout - moving to next waypoint"));
		EndAnswer();
	}
}

void UAnswerStateSystem::Tick_AnswerReply(float DeltaTime)
{
	if (!OwnerDasan)
		return;

	ReplyTimer += DeltaTime;

	// 자동 종료 모드일 경우 시간이 지나면 자동으로 Listen으로 복귀
	if (bAutoEndAnswer && ReplyTimer >= ReplyDuration)
	{
		OnAnswerFinished();
	}
}

// 블루프린트 호출 함수들
void UAnswerStateSystem::OnQuestionDetected()
{
	if (!OwnerDasan || !OwnerDasan->HasAuthority())
		return;

	PRINTLOG( TEXT("[AnswerState] Question detected!"));

	// Listen 상태에서만 Reply로 전환
	if (CurState == EAnswerState::AnswerListen)
		CurState = EAnswerState::AnswerReply;
}

void UAnswerStateSystem::OnAnswerFinished()
{
	if (!OwnerDasan || !OwnerDasan->HasAuthority())
		return;

	PRINTLOG( TEXT("[AnswerState] Answer finished - back to listening"));

	// Reply에서 다시 Listen으로
	CurState = EAnswerState::AnswerListen;
	ReplyTimer = 0.0f;
	ListenTimer = 0.0f;
}

void UAnswerStateSystem::EndAnswer()
{
	if (!OwnerDasan || !OwnerDasan->HasAuthority())
		return;

	PRINTLOG( TEXT("[AnswerState] Ending answer session"));

	// Answer 종료 후 다음 퀘스트로 이동
	CurState = EAnswerState::AnswerEnd;
	OwnerDasan->TransitionToState(EDasanState::Tour);
}

bool UAnswerStateSystem::CanStartAnswer(const FString& PlayerName, FString& OutReason) const
{
	if (!OwnerDasan)
	{
		OutReason = TEXT("다산을 찾을 수 없습니다.");
		return false;
	}

	// 이미 Answer 상태이고, 다른 플레이어가 질문 중이면 거부
	if (OwnerDasan->DasanState == EDasanState::Answer && !CurrentQuestionerName.IsEmpty())
	{
		if (CurrentQuestionerName != PlayerName)
		{
			OutReason = FString::Printf(TEXT("다산은 %s에게 답변중입니다."), *CurrentQuestionerName);
			return false;
		}
	}

	return true;
}

bool UAnswerStateSystem::TryStartAnswer(const FString& PlayerName)
{
	if (!OwnerDasan || !OwnerDasan->HasAuthority())
		return false;

	FString Reason;
	if (!CanStartAnswer(PlayerName, Reason))
	{
		// 거부 메시지를 요청한 플레이어에게 표시
		PRINTLOG(TEXT("[AnswerSystem] Answer 시작 거부: %s"), *Reason);

		// 모든 클라이언트에 다이얼로그 표시 (Toast는 로컬 플레이어만 보임)
		UDialogManager::Toast(GetWorld(), Reason);

		return false;
	}

	// Answer 시작 성공
	CurrentQuestionerName = PlayerName;
	PRINTLOG(TEXT("[AnswerSystem] %s님의 질문 시작"), *PlayerName);

	// 현재 Dasan 메인 상태가 Answer가 아닐 때만 처리
	if (OwnerDasan->DasanState != EDasanState::Answer)
	{
		// 이전 상태 저장
		PreviousMainState = OwnerDasan->DasanState;
		PRINTLOG(TEXT("[AnswerSystem] 이전 메인 상태 저장: %s"),
			*ENUM_TO_NAME(EDasanState, PreviousMainState));

		// AI 이동 중지
		if (OwnerDasan->DasanAicontrol)
		{
			OwnerDasan->DasanAicontrol->StopMovement();
			PRINTLOG(TEXT("[AnswerSystem] AI 이동 중지"));
		}

		// Answer 상태로 전환
		OwnerDasan->TransitionToState(EDasanState::Answer);
	}

	return true;
}

void UAnswerStateSystem::FinishAnswer()
{
	if (!OwnerDasan || !OwnerDasan->HasAuthority())
		return;

	PRINTLOG(TEXT("[AnswerSystem] %s님의 질문 종료"), *CurrentQuestionerName);

	// UI 위젯 숨기기 (다산이 듣기 종료)
	if (BroadcastManager)
	{
		BroadcastManager->SendDasanListening(false, TEXT(""));
		PRINTLOG(TEXT("[AnswerState] SendDasanListening(false)"));
	}

	CurrentQuestionerName.Empty();

	// 이전 상태로 복귀
	if (PreviousMainState != EDasanState::Answer)
	{
		PRINTLOG(TEXT("[AnswerSystem] 이전 상태로 복귀: %s"), *ENUM_TO_NAME(EDasanState, PreviousMainState));
		OwnerDasan->TransitionToState(PreviousMainState);
	}
}

void UAnswerStateSystem::OnAudioCaptureChanged(bool bRecording)
{
	if (!OwnerDasan || !OwnerDasan->HasAuthority())
		return;

	if (bRecording)
	{
		// 음성 녹음 시작 - 플레이어 이름 가져오기
		PRINTLOG(TEXT("[AnswerSystem] 음성 녹음 시작 감지"));

		// 첫 번째 플레이어 가져오기
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (!PC || !PC->GetPawn())
		{
			PRINTLOG(TEXT("[AnswerSystem] 플레이어를 찾을 수 없습니다."));
			return;
		}

		// APlayerActor에서 실제 플레이어 표시 이름 가져오기
		FString PlayerName = TEXT("Unknown");
		if (auto PlayerActor = Cast<APlayerActor>(PC->GetPawn()))
		{
			PlayerName = PlayerActor->GetPlayerDisplayName();
		}

		PRINTLOG(TEXT("[AnswerSystem] 플레이어 이름: %s"), *PlayerName);

		// Answer 시작 시도
		TryStartAnswer(PlayerName);
	}
	else
	{
		// 음성 녹음 종료 - TTS 대기 중
		PRINTLOG(TEXT("[AnswerSystem] 음성 녹음 종료 감지 (Answer 상태 유지 - TTS 대기)"));
	}
}

void UAnswerStateSystem::OnTTSPlaybackFinished()
{
	if (!OwnerDasan || !OwnerDasan->HasAuthority())
		return;

	PRINTLOG(TEXT("[AnswerSystem] TTS 재생 완료 - FinishAnswer 호출"));

	// Answer 완료 처리
	FinishAnswer();
}