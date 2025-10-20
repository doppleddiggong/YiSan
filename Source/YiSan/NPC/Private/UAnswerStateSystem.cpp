// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UAnswerStateSystem.h"
#include "ADasanActor.h"
#include "GameLogging.h"

UAnswerStateSystem::UAnswerStateSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAnswerStateSystem::InitSystem(ADasanActor* InOwner)
{
	OwnerDasan = InOwner;
	PrevState = EAnswerState::AnswerListen;
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
	PRINTLOG( TEXT("[AnswerState] Enter AnswerListen - Waiting for questions..."));
	ListenTimer = 0.0f;
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