// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UTourStateSystem.h"

#include "ABuilding.h"
#include "ADasanActor.h"
#include "AIController.h"
#include "APlayerActor.h"
#include "AYisanGameState.h"
#include "FBuildingAssetData.h"
#include "FComponentHelper.h"
#include "GameLogging.h"
#include "UBroadcastManager.h"
#include "UBuildingDetailData.h"
#include "UGameDataManager.h"

#include "Components/AudioComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Sound/SoundCue.h"

// 2초에 한 번 검사
static float CheckInterval = 4.0f;

UTourStateSystem::UTourStateSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTourStateSystem::InitSystem(ADasanActor* InOwner)
{
	OwnerDasan = InOwner;
	PrevState = ETourState::None;

	// BroadcastManager 초기화
	BroadcastManager = UBroadcastManager::Get(GetWorld());
	if (!BroadcastManager)
	{
		PRINTLOG(TEXT("[TourState] BroadcastManager를 찾을 수 없습니다!"));
	}
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
			case ETourState::TourExplain: Enter_TourExplain(); break;
			case ETourState::TourEnd: Enter_TourEnd(); break;
			default:  break;
		}
		
		PrevState = CurState;
	}

	switch (CurState)
	{
		case ETourState::TourMove: Tick_TourMove(DeltaTime); break;
		case ETourState::TourWait: Tick_TourWait(DeltaTime); break;
		case ETourState::TourExplain: Tick_TourExplain(DeltaTime); break;
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

void UTourStateSystem::Enter_TourExplain()
{
	PRINTLOG( TEXT("[TourState] Enter TourExplain"));

	auto building = OwnerDasan->GetCurTargetBuilding();

	// 건물 에셋 정보 데이터 가져오기
	FBuildingAssetData AssetData;
	if ( UGameDataManager::Get(GetWorld())->GetBuildingAssetData( building->BuildingType, AssetData) )
	{
		UBuildingDetailData* DetailAsset = AssetData.BuildingDetailDataAsset.LoadSynchronous();

		if ( DetailAsset )
		{
			TSoftObjectPtr<USoundCue> LoadedCue;
			if ( DetailAsset->LoadSoundCue(LoadedCue) && LoadedCue.Get() )
			{
				if (PlayingSound && PlayingSound->IsPlaying())
					PlayingSound->Stop();
				PlayingSound = UGameplayStatics::SpawnSound2D(GetWorld(), LoadedCue.Get());
			}
		}
	}
}

void UTourStateSystem::Enter_TourEnd()
{
	PRINTLOG( TEXT("[TourState] Tour End - All waypoints completed"));

	// 모든 투어 웨이포인트 완료
	// 필요 시 추가 처리 (예: 다시 처음부터 시작, 또는 대기 상태)
	if (auto GS = GetWorld()->GetGameState<AYisanGameState>())
	{
		GS->ServerRPC_BroadcastToastMessage(TEXT("모든 투어 일정이 종료되었습니다"));
	}
}

// Tick 함수들
void UTourStateSystem::Tick_TourMove(float DeltaTime)
{
	if (!OwnerDasan)
		return;

	// 웨이포인트로 이동
	// 현재 목표 건물이 없으면 찾기
	if (!OwnerDasan->GetCurTargetBuilding())
		OwnerDasan->UpdateTargetBuilding( OwnerDasan->FindCurTargetBuilding());

	// 여전히 없으면 리턴
	if (!OwnerDasan->GetCurTargetBuilding())
	{
		PRINTLOG( TEXT(" No target building found"));
		return;
	}

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(OwnerDasan->GetCurTargetBuilding());
	MoveRequest.SetAcceptanceRadius(250.0f);
	MoveRequest.SetUsePathfinding(true);
			
	OwnerDasan->DasanAicontrol->MoveTo(MoveRequest);

	
	// 웨이포인트 도착 체크
	if (OwnerDasan->IsNearTargetBuilding())
	{
		PRINTLOG(TEXT("[TourState] 웨이포인트 도착 → TourExplain"));
		CurState = ETourState::TourExplain;
		return;
	}
	
	// 주변 플레이어 검사 간격
	WaitTimer += DeltaTime;

	if (WaitTimer >= CheckInterval)
	{
		WaitTimer = 0.0f;

		if (IsAllPlayersNearby())
		{
			PRINTLOG(TEXT("[TourState] 플레이어 전원 근처 → TourWait"));
			WaitTimer = 0.0f;
			CurState = ETourState::TourWait;
		}
		else
		{
			PRINTLOG(TEXT("[TourState] 아직 더 모여야 함"));
		}
	}
}

void UTourStateSystem::Tick_TourWait(float DeltaTime)
{
	if (!OwnerDasan)
		return;

	// 주기적 체크 타이머 누적
	WaitTimer += DeltaTime;

	if (WaitTimer >= CheckInterval)
	{
		WaitTimer = 0.0f; // 리셋

		if (IsAllPlayersNearby())
		{
			CurState = ETourState::TourMove;

			PRINTLOG(TEXT("[TourState] 모든 플레이어 근처 → TourMove 전환"));
		}
		else
		{
			PRINTLOG(TEXT("[TourState] 일부 플레이어 미도달"));
		}
	}
}

void UTourStateSystem::Tick_TourExplain(float DeltaTime)
{
	if (!OwnerDasan)
		return;

	WaitTimer += DeltaTime;

	// 플레이어들이 근처에 있는지 확인
	if (IsAllPlayersNearby() && WaitTimer >= WaitTimeBeforeTour)
	{
		// 관광 시간 카운트다운
		TourViewTimer += DeltaTime;

		// 남은 시간 계산
		int32 RemainingTime = FMath::CeilToInt(TourViewDuration - TourViewTimer);
		RemainingTime = FMath::Max(0, RemainingTime);

		// 1초 단위로 메시지 전송 (너무 자주 보내지 않도록)
		if (RemainingTime != LastReportedTime && RemainingTime >= 0)
		{
			FString ChatMessageText = FString::Printf(TEXT("다음 투어까지 남은 시간: %d초"), RemainingTime);
			PRINTLOG(TEXT("[TourState] %s"), *ChatMessageText);

			if (auto GS = GetWorld()->GetGameState<AYisanGameState>())                                                                                                                                               
			    GS->ServerRPC_BroadcastToastMessage(ChatMessageText);                                                           

			LastReportedTime = RemainingTime;
		}

		// 관광 시간이 끝나면 다음 건물로 이동
		if (TourViewTimer >= TourViewDuration)
		{
			PRINTLOG(TEXT("[TourState] 관광 시간 종료 - 다음 건물로 이동"));

			// 다음 퀘스트(건물)로 이동
			OwnerDasan->NextQuest();

			// 타이머 리셋
			WaitTimer = 0.0f;
			TourViewTimer = 0.0f;
			LastReportedTime = -1;
		}
	}
}

bool UTourStateSystem::IsAllPlayersNearby() const
{
	if (!OwnerDasan)
		return false;

	// ACharacter 기반 플레이어 모두 수집
	auto Players = FComponentHelper::GetAllOfClass<APlayerActor>(GetWorld());
	FVector DasanLocation = OwnerDasan->GetActorLocation();

	for (auto Player : Players)
	{
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