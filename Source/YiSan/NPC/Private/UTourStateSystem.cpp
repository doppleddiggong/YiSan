// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UTourStateSystem.h"

#include "ABuilding.h"
#include "ADasanActor.h"
#include "FComponentHelper.h"
#include "GameLogging.h"
#include "UQuestManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AIController.h"

UTourStateSystem::UTourStateSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTourStateSystem::InitSystem(ADasanActor* InOwner)
{
	OwnerDasan = InOwner;
	PrevState = ETourState::None;
	CurState = ETourState::None;
	
	PRINTLOG(TEXT("[TourSystem] 시스템 초기화 완료"));
}

void UTourStateSystem::SetTourState(const ETourState InState)
{
	if (CurState == InState)
		return;
		
	PRINTLOG(TEXT("[TourSystem] 상태 변경: %s -> %s"),
		*ENUM_TO_NAME(ETourState, CurState),
		*ENUM_TO_NAME(ETourState, InState));
	
	CurState = InState;

	// 상태가 변경되었으므로 위젯 업데이트
	if (OwnerDasan)
	{
		OwnerDasan->UpdateWidgetState();
	}
}

bool UTourStateSystem::IsUpdateEnble()
{
	if (OwnerDasan == nullptr)
		return false;

	if (OwnerDasan->HasAuthority() == false)
		return false;
	
	return true;
}

void UTourStateSystem::UpdateTick(float DeltaTime)
{
	if (!IsUpdateEnble())
		return;
	
	if (PrevState != CurState)
	{
		switch (CurState)
		{
			case ETourState::TourMove: Enter_TourMove(); break;
			case ETourState::TourWait: Enter_TourWait(); break;
			case ETourState::TourEnd: Enter_TourEnd(); break;
			default: break;
		}
		
		PrevState = CurState;
	}

	switch (CurState)
	{
		case ETourState::TourMove: Tick_TourMove(DeltaTime); break;
		case ETourState::TourWait: Tick_TourWait(DeltaTime); break;
		default: break;
	}
}

// Enter 함수들
void UTourStateSystem::Enter_TourMove()
{
	PRINTLOG(TEXT("[TourState] Enter TourMove"));
	WaitTimer = 0.0f;
	
	// AI Controller를 통한 이동은 ADasanActor에서 처리됨
}

void UTourStateSystem::Enter_TourWait()
{
	PRINTLOG(TEXT("[TourState] Enter TourWait"));
	WaitTimer = 0.0f;

	// 캐릭터 정지
	if (OwnerDasan)
	{
		UCharacterMovementComponent* Movement = OwnerDasan->GetCharacterMovement();
		if (Movement)
		{
			Movement->StopMovementImmediately();
		}
		
		// AI 이동도 중지
		if (OwnerDasan->DasanAicontrol)
		{
			OwnerDasan->DasanAicontrol->StopMovement();
		}
	}
}

void UTourStateSystem::Enter_TourEnd()
{
	PRINTLOG(TEXT("[TourState] Enter TourEnd"));

	if (!OwnerDasan)
		return;

	// 이동 중지
	if (OwnerDasan->DasanAicontrol)
	{
		OwnerDasan->DasanAicontrol->StopMovement();
	}

	// 다음 퀘스트가 있는지 확인
	UQuestManager* QuestManager = OwnerDasan->QuestManager;
	
	if (QuestManager && QuestManager->IsHasQuest() && QuestManager->GetCurTarget() != EBuildingType::None)
	{
		PRINTLOG(TEXT("[TourState] 다음 퀘스트로 이동"));
		
		// 다음 퀘스트로 이동
		OwnerDasan->NextQuest();
	}
	else
	{
		// 모든 퀘스트 완료 - Answer 상태로 전환
		PRINTLOG(TEXT("[TourState] 모든 퀘스트 완료 - Answer 상태로 전환"));
		OwnerDasan->TransitionToState(EDasanState::Answer);
	}
}

// Tick 함수들
void UTourStateSystem::Tick_TourMove(float DeltaTime)
{
	if (!OwnerDasan)
		return;

	// 이동은 AI Controller가 자동으로 처리
	// 도착 감지는 OnMoveCompleted 콜백에서 처리됨
}

void UTourStateSystem::Tick_TourWait(float DeltaTime)
{
	if (!OwnerDasan)
		return;

	WaitTimer += DeltaTime;
	
	// 플레이어 대기는 ADasanActor의 UpdateTourState에서 처리됨
}

// 유틸리티 함수들 - 더 이상 사용하지 않지만 호환성을 위해 유지
bool UTourStateSystem::IsNearWaypoint() const
{
	if (!OwnerDasan || !OwnerDasan->GetCurTargetBuilding())
		return false;

	float Distance = FVector::Dist(
		OwnerDasan->GetActorLocation(), 
		OwnerDasan->GetCurTargetBuilding()->GetActorLocation()
	);

	return Distance <= WaypointReachDistance;
}

bool UTourStateSystem::IsAllPlayersNearby() const
{
	if (!OwnerDasan)
		return false;

	// ACharacter 기반 플레이어 모두 수집
	auto Players = FComponentHelper::GetAllOfClass<ACharacter>(GetWorld());
	FVector DasanLocation = OwnerDasan->GetActorLocation();

	for (ACharacter* Player : Players)
	{
		// 자신은 제외
		if (Player == OwnerDasan)
			continue;

		// 거리 체크
		float Distance = FVector::Dist(DasanLocation, Player->GetActorLocation());
		if (Distance > PlayerDetectionRadius)
		{
			return false;
		}
	}

	return true;
}