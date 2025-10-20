// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UExplainStateSystem.h"
#include "ADasanActor.h"
#include "GameLogging.h"

UExplainStateSystem::UExplainStateSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UExplainStateSystem::InitSystem(ADasanActor* InOwner)
{
	OwnerDasan = InOwner;
	PrevState = EExplainState::ExplainWait;
}

bool UExplainStateSystem::IsUpdateEnble()
{
	if ( OwnerDasan == nullptr)
		return false;

	if ( OwnerDasan->HasAuthority() == false)
		return false;
	return true;
}

void UExplainStateSystem::UpdateTick(float DeltaTime)
{
	if ( !IsUpdateEnble())
		return;

	if ( PrevState != CurState )
	{
		switch (CurState)
		{
			case EExplainState::ExplainWait: Enter_ExplainWait(); break;
			case EExplainState::ExplainIng: Enter_ExplainIng(); break;
			default: break;
		}
		PrevState = CurState;
	}

	switch (CurState)
	{
		case EExplainState::ExplainWait: Tick_ExplainWait(DeltaTime); break;
		case EExplainState::ExplainIng: Tick_ExplainIng(DeltaTime); break;
		default: break;
	}
}

// Enter 함수들
void UExplainStateSystem::Enter_ExplainWait()
{
	PRINTLOG( TEXT("[ExplainState] Enter ExplainWait"));
	ExplainTimer = 0.0f;
}

void UExplainStateSystem::Enter_ExplainIng()
{
	PRINTLOG( TEXT("[ExplainState] Enter ExplainIng"));
	ExplainTimer = 0.0f;

	// [TODO] 여기서 블루프린트 이벤트 호출 가능
	// 예: 음성 재생, 자막 표시 등
}

// Tick 함수들
void UExplainStateSystem::Tick_ExplainWait(float DeltaTime)
{
	// 대기 중 - 특별한 동작 없음
}

void UExplainStateSystem::Tick_ExplainIng(float DeltaTime)
{
	if (!OwnerDasan)
		return;

	ExplainTimer += DeltaTime;

	// 자동 종료 모드일 경우 시간이 지나면 자동으로 완료
	if (bAutoFinishExplain && ExplainTimer >= ExplainDuration)
	{
		OnExplainFinished();
	}
}

void UExplainStateSystem::OnExplainFinished()
{
	if (!OwnerDasan || !OwnerDasan->HasAuthority())
		return;

	PRINTLOG( TEXT("[ExplainState] Explain Finished"));

	OwnerDasan->TransitionToState(EDasanState::Tour);
	ExplainTimer = 0.0f;
}