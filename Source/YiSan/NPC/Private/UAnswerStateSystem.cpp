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

	DOREPLIFETIME(UAnswerStateSystem, CurQuestionerName);
	DOREPLIFETIME(UAnswerStateSystem, CurState);
}

void UAnswerStateSystem::BeginPlay()
{
	Super::BeginPlay();
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
		BroadcastManager->OnAudioCapture.AddDynamic(this, &UAnswerStateSystem::OnAudioCapture);
		BroadcastManager->OnVoiceAudioFinished.AddDynamic(this, &UAnswerStateSystem::OnTTSFinished);
		BroadcastManager->OnAnswerReply.AddDynamic(this, &UAnswerStateSystem::OnAnswerReply);
	}
}

void UAnswerStateSystem::SetCurState(EAnswerState InState)
{
	CurState = InState;

	// 상태가 변경되었으므로 위젯 업데이트
	if (OwnerDasan)
	{
		OwnerDasan->UpdateWidgetState();
	}
}


void UAnswerStateSystem::OnRep_CurState()
{
	if (OwnerDasan)
	{
		OwnerDasan->UpdateWidgetState();
	}
}

// 블루프린트 호출 함수들
void UAnswerStateSystem::OnAnswerReply()
{
	if (!OwnerDasan || !OwnerDasan->HasAuthority())
		return;

	PRINTLOG( TEXT("[AnswerState] OnAnswerReply"));

	// Listen 상태에서만 Reply로 전환
	if (CurState == EAnswerState::AnswerListen)
	{
		this->SetCurState(EAnswerState::AnswerReply);
	}
}


bool UAnswerStateSystem::CanStartAnswer(const FString& PlayerName, FString& OutReason) const
{
	if (!OwnerDasan)
	{
		OutReason = TEXT("다산을 찾을 수 없습니다.");
		return false;
	}

	// 이미 Answer 상태이고, 다른 플레이어가 질문 중이면 거부
	if (OwnerDasan->DasanState == EDasanState::Answer && !CurQuestionerName.IsEmpty())
	{
		if (CurQuestionerName != PlayerName)
		{
			OutReason = FString::Printf(TEXT("다산은 %s에게 답변중입니다."), *CurQuestionerName);
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
		if (auto DM = UDialogManager::Get(GetWorld()))
			DM->ShowToast(Reason);

		return false;
	}

	// Answer 시작 성공
	CurQuestionerName = PlayerName;
	PRINTLOG(TEXT("[AnswerSystem] %s님의 질문 시작"), *PlayerName);

	// 타임아웃 타이머 시작 (예외 상황 대비)
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			AnswerTimeoutTimer,
			this,
			&UAnswerStateSystem::OnAnswerTimeout,
			AnswerTimeoutDuration,
			false
		);
		PRINTLOG(TEXT("[AnswerSystem] 답변 타임아웃 타이머 시작 (%.1f초)"), AnswerTimeoutDuration);
	}

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

	PRINTLOG(TEXT("[AnswerSystem] %s님의 질문 종료"), *CurQuestionerName);

	// 타임아웃 타이머 정리
	if (GetWorld() && AnswerTimeoutTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AnswerTimeoutTimer);
		PRINTLOG(TEXT("[AnswerSystem] 답변 타임아웃 타이머 정리"));
	}

	// UI 위젯 숨기기 (다산이 듣기 종료)
	if (BroadcastManager)
	{
		BroadcastManager->SendAskListening(false, TEXT(""));
		PRINTLOG(TEXT("[AnswerState] SendDasanListening(false)"));
	}

	CurQuestionerName.Empty();

	// 이전 상태로 복귀
	if (PreviousMainState != EDasanState::Answer)
	{
		PRINTLOG(TEXT("[AnswerSystem] 이전 상태로 복귀: %s"), *ENUM_TO_NAME(EDasanState, PreviousMainState));
		OwnerDasan->TransitionToState(PreviousMainState);
	}
}

void UAnswerStateSystem::OnAnswerTimeout()
{
	if (!OwnerDasan || !OwnerDasan->HasAuthority())
		return;

	PRINTLOG(TEXT("[AnswerSystem] 답변 타임아웃 발생 - 강제 종료"));

	// 타임아웃 메시지 표시
	if (auto DM = UDialogManager::Get(GetWorld()))
		DM->ShowToast(TEXT("질문 시간이 초과되어 대화가 종료되었습니다."));

	// 답변 종료 처리
	FinishAnswer();
}

bool UAnswerStateSystem::IsAnswerSessionActive() const
{
	return !CurQuestionerName.IsEmpty();
}

void UAnswerStateSystem::OnAudioCapture(bool bRecording)
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

void UAnswerStateSystem::OnTTSFinished()
{
	if (!OwnerDasan || !OwnerDasan->HasAuthority())
		return;

	PRINTLOG(TEXT("[AnswerSystem] TTS 재생 완료 - FinishAnswer 호출"));

	// Answer 완료 처리
	FinishAnswer();
}