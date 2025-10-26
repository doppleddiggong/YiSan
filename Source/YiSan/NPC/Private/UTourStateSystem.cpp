// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UTourStateSystem.h"

#include "ABuilding.h"
#include "ADasanActor.h"
#include "FComponentHelper.h"
#include "GameLogging.h"
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
}

void UTourStateSystem::SetTourState(const ETourState InState)
{
	CurState = InState;

	// 상태가 변경되었으므로 위젯 업데이트
	if (OwnerDasan)
	{
		OwnerDasan->UpdateWidgetState();
	}
}

bool UTourStateSystem::IsUpdateEnble()
{
	if ( OwnerDasan == nullptr)
		return false;

	if ( OwnerDasan->HasAuthority() == false)
		return false;
	return true;
}

void UTourStateSystem::UpdateTick(float DeltaTime )
{
	if ( !IsUpdateEnble())
		return;
	
	if (PrevState != CurState)
	{
		switch (CurState)
		{
			case ETourState::TourMove: Enter_TourMove(); break;
			case ETourState::TourWait: Enter_TourWait(); break;
			case ETourState::TourEnd: Enter_TourEnd(); break;
			default:  break;
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
	PRINTLOG( TEXT("[TourState] Enter TourMove"));
	WaitTimer = 0.0f;
}

void UTourStateSystem::Enter_TourWait()
{
	PRINTLOG( TEXT("[TourState] Enter TourWait"));
	WaitTimer = 0.0f;

	// 캐릭터 정지
	if (OwnerDasan)
	{
		UCharacterMovementComponent* Movement = OwnerDasan->GetCharacterMovement();
		if (Movement)
		{
			Movement->StopMovementImmediately();
		}
	}
}

void UTourStateSystem::Enter_TourEnd()
{
	PRINTLOG( TEXT("[TourState] Tour End"));

	// 투어 종료 후 다음 메인 상태로 전환
	if (OwnerDasan)
		OwnerDasan->TransitionToState(EDasanState::Explain);
}

// Tick 함수들
void UTourStateSystem::Tick_TourMove(float DeltaTime)
{
	if (!OwnerDasan)
		return;

	// 웨이포인트로 이동
	MoveToCurWaypoint();

	// 웨이포인트 도착 체크
	if (IsNearWaypoint())
	{
		CurState = ETourState::TourWait;
	}
}

void UTourStateSystem::Tick_TourWait(float DeltaTime)
{
	if (!OwnerDasan)
		return;

	WaitTimer += DeltaTime;

	// 플레이어들이 근처에 있는지 확인
	if ( IsAllPlayersNearby() && WaitTimer >= WaitTimeBeforeExplain)
	{
		// 설명 상태로 전환
		OwnerDasan->TransitionToState(EDasanState::Explain);
		WaitTimer = 0.0f;
	}
}

// 유틸리티 함수들
void UTourStateSystem::MoveToCurWaypoint()
{
	if (!OwnerDasan)
		return;

	// 현재 목표 건물이 없으면 찾기
	if (!OwnerDasan->GetCurTargetBuilding())
		OwnerDasan->UpdateTargetBuilding( OwnerDasan->FindCurTargetBuilding());

	// 여전히 없으면 리턴
	if (!OwnerDasan->GetCurTargetBuilding())
	{
		PRINTLOG( TEXT(" No target building found"));
		return;
	}

	// AI Controller가 있으면 AI MoveTo 사용
	if (OwnerDasan->DasanAicontrol)
	{
		// AI MoveTo로 이동 - NavMesh 기반 경로 탐색
		// 이 함수는 Tick마다 호출되지만, AI MoveTo는 한 번만 시작됨
		// 실제 이동은 UpdateTourState 또는 OnMoveCompleted에서 관리됨
		return;
	}

	// AI Controller가 없으면 직접 이동 방식 사용 (기존 코드)
	// 목표 방향 계산
	const FVector CurLoc  = OwnerDasan->GetActorLocation();
	const FVector TargetLoc  = OwnerDasan->GetCurTargetBuilding()->GetActorLocation();
	const FVector Direction = (TargetLoc - CurLoc).GetSafeNormal();

	// 회전
	const FRotator TargetRotation = Direction.Rotation();
	
	OwnerDasan->SetActorRotation(FMath::RInterpTo(
		OwnerDasan->GetActorRotation(),
		TargetRotation,
		GetWorld()->GetDeltaSeconds(),
		5.0f
	));

	// 직선 이동
	if (auto Movement = OwnerDasan->GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = MoveSpeed;
		OwnerDasan->AddMovementInput(Direction, 1.0f);
	}
}

bool UTourStateSystem::IsNearWaypoint() const
{
	if (!OwnerDasan || !OwnerDasan->GetCurTargetBuilding())
		return false;

	float Distance = FVector::Dist( OwnerDasan->GetActorLocation(), OwnerDasan->GetCurTargetBuilding()->GetActorLocation() );

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
			// 하나라도 기준 밖이면 전체 실패
			return false;
		}
	}

	// 모든 플레이어가 범위 안에 있음
	return true;
}