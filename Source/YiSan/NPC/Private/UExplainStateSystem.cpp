// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#include "UExplainStateSystem.h"
#include "ADasanActor.h"
#include "GameLogging.h"
#include "ABuilding.h"
#include "EBuildingType.h"
#include "Macro.h"
#include "UQuestManager.h"
#include "UTourStateSystem.h"

UExplainStateSystem::UExplainStateSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UExplainStateSystem::InitSystem(ADasanActor* InOwner)
{
	OwnerDasan = InOwner;
	PrevState = EExplainState::ExplainWait;
	CurState = EExplainState::ExplainWait;
	
	PRINTLOG(TEXT("[ExplainSystem] 시스템 초기화 완료"));
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
		PRINTLOG(TEXT("[ExplainSystem] 상태 변경: %s -> %s"), 
			*ENUM_TO_NAME(EExplainState, PrevState),
			*ENUM_TO_NAME(EExplainState, CurState));
			
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
	PRINTLOG(TEXT("[ExplainState] Enter ExplainWait"));
	ExplainTimer = 0.0f;
}

void UExplainStateSystem::Enter_ExplainIng()
{
	PRINTLOG(TEXT("[ExplainState] Enter ExplainIng"));
	ExplainTimer = 0.0f;

	if (OwnerDasan)
	{
		ABuilding* TargetBuilding = OwnerDasan->GetCurTargetBuilding();
		if (TargetBuilding)
		{
			FString BuildingTypeName = ENUM_TO_NAME(EBuildingType, TargetBuilding->BuildingType);
			PRINTLOG(TEXT("지금부터 %s 에 대한 설명을 시작함."), *BuildingTypeName);
		}
	}
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

	// 1초마다 로그 출력
	static float LogTimer = 0.0f;
	LogTimer += DeltaTime;
	if (LogTimer >= 1.0f)
	{
		PRINTLOG(TEXT("[ExplainIng] 설명 중... (%.1f / %.1f초)"), ExplainTimer, ExplainDuration);
		LogTimer = 0.0f;
	}

	// 자동 종료 모드일 경우 시간이 지나면 자동으로 완료
	if (bAutoFinishExplain && ExplainTimer >= ExplainDuration)
	{
		PRINTLOG(TEXT("[ExplainIng] 설명 시간 종료 - 자동 완료"));
		OnExplainFinished();
	}
}

void UExplainStateSystem::OnExplainFinished()
{
	if (!OwnerDasan || !OwnerDasan->HasAuthority())
		return;
	
	PRINTLOG(TEXT("[ExplainState] Explain Finished"));

	// 다음 퀘스트로 이동
	if (OwnerDasan->QuestManager && OwnerDasan->QuestManager->IsHasQuest())
	{
		OwnerDasan->NextQuest();
	}
	
	// 메인 상태를 Tour로 변경
	OwnerDasan->TransitionToState(EDasanState::Tour); 

	// Explain 시스템은 다시 대기 상태로
	ExplainTimer = 0.0f;
	SetExplainState(EExplainState::ExplainWait);
}