// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UTourStateSystem.h"

#include "ABuilding.h"
#include "ADasanActor.h"
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
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"

#include "DrawDebugHelpers.h"
#include "UCommonFunctionLibrary.h"
#include "UGameSoundManager.h"

// 2초에 한 번 검사
static float CheckInterval = 2.0f;

UTourStateSystem::UTourStateSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UTourStateSystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTourStateSystem, CurState);
}

void UTourStateSystem::OnRep_CurState()
{
	PRINTLOG(TEXT("[TourSystem] OnRep_CurState - New State: %s (Client)"), *ENUM_TO_NAME(ETourState, CurState));

	// 클라이언트에서 상태가 복제되었으므로 위젯 업데이트
	if (OwnerDasan)
	{
		OwnerDasan->UpdateWidgetState();
	}
	else
	{
		PRINTLOG(TEXT("[TourSystem] OnRep_CurState - OwnerDasan is nullptr (InitSystem not called yet)"));
	}
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

	if (bEnableDebugDraw)
	{
		if (auto World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DebugTimerHandle);
			World->GetTimerManager().SetTimer(
				DebugTimerHandle,
				this,
				&UTourStateSystem::DrawPlayerRadiusDebug,
				0.15f,
				true);
		}
	}
}

void UTourStateSystem::SetTourState(const ETourState InState)
{
	const ETourState OldState = CurState;
	CurState = InState;

	PRINTLOG(TEXT("[TourSystem] SetTourState - %s → %s (Authority: %s)"),
		*ENUM_TO_NAME(ETourState, OldState),
		*ENUM_TO_NAME(ETourState, InState),
		OwnerDasan && OwnerDasan->HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));

	// 상태가 변경되었으므로 위젯 업데이트
	if (OwnerDasan)
	{
		OwnerDasan->UpdateWidgetState();
	}
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

	// Enter 시에도 이동 명령 실행
	if (OwnerDasan && OwnerDasan->DasanAicontrol && OwnerDasan->GetCurTargetBuilding())
	{
		FVector StartPos = OwnerDasan->GetActorLocation();
		FVector TargetPos = OwnerDasan->GetCurTargetBuilding()->GetActorLocation();
		float Distance = FVector::Dist(StartPos, TargetPos);

		PRINTLOG(TEXT("[TourState] MoveTo 시작 - From: (%.1f, %.1f, %.1f) To: (%.1f, %.1f, %.1f), Distance: %.1f"),
			StartPos.X, StartPos.Y, StartPos.Z,
			TargetPos.X, TargetPos.Y, TargetPos.Z,
			Distance);

		// NavMesh로 목표 위치 투영
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSys)
		{
			FNavLocation TargetNavLoc;
			bool bTargetOnNavMesh = NavSys->ProjectPointToNavigation(TargetPos, TargetNavLoc, FVector(5000, 5000, 5000));

			if (bTargetOnNavMesh)
			{
				float ProjectionDist = FVector::Dist(TargetPos, TargetNavLoc.Location);
				PRINTLOG(TEXT("[TourState] 목표 NavMesh 투영: 원본 Z=%.1f → NavMesh Z=%.1f (높이차: %.1f)"),
					TargetPos.Z, TargetNavLoc.Location.Z, ProjectionDist);

				auto Result = OwnerDasan->AIMoveToLoc(TargetNavLoc.Location, 250.0f, true);
				PRINTLOG(TEXT("[TourState] MoveTo 결과: %d (NavMesh 위치 사용)"), (int32)Result.Code);
			}
			else
			{
				PRINTLOG(TEXT("[TourState] 경고: 목표 위치를 NavMesh에 투영 실패! Actor 위치로 시도"));

				auto Result = OwnerDasan->AIMoveToActor( OwnerDasan->GetCurTargetBuilding(), 250.0f, true );
				PRINTLOG(TEXT("[TourState] MoveTo 결과: %d (Actor 위치 사용)"), (int32)Result.Code);
			}
		}
		else
		{
			PRINTLOG(TEXT("[TourState] 경고: NavigationSystem 없음! Actor 위치로 시도"));
			auto Result = OwnerDasan->AIMoveToActor( OwnerDasan->GetCurTargetBuilding(), 250.0f, true );
			PRINTLOG(TEXT("[TourState] MoveTo 결과: %d"), (int32)Result.Code);
		}
	}
}

void UTourStateSystem::Enter_TourWait()
{
	PRINTLOG( TEXT("[TourState] Enter TourWait"));
	WaitTimer = 0.0f;
	ForcedGatherTimer = 0.0f;
	LastReportedGatherTime = -1;

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

	// 설명 출력 상태 초기화
	ExplainLines.Empty();
	ExplainLineIndex = 0;
	ExplainLineTimer = 0.0f;
	PostExplainWaitTimer = 0.0f;
	bExplainCompleted = false;

	// explain 시작하는거 알리기 위함
	if (OwnerDasan)
	{
		OwnerDasan->StartExplainAnim();
	}

	auto building = OwnerDasan->GetCurTargetBuilding();

	// 건물 정보 데이터 가져오기
	FBuildingData BuildingData;
	if (UGameDataManager::Get(GetWorld())->GetBuildingData(building->BuildingType, BuildingData))
	{
		// explain 필드를 줄바꿈으로 분할
		FString ExplainText = BuildingData.explain;
		if (!ExplainText.IsEmpty())
		{
			// \n으로 분할
			ExplainText.ParseIntoArray(ExplainLines, TEXT("\n"), true);
			PRINTLOG(TEXT("[TourState] 설명 라인 수: %d"), ExplainLines.Num());
		}
		else
		{
			PRINTLOG(TEXT("[TourState] 경고: 건물 설명이 비어있습니다!"));
		}
	}

	// 음성 재생 로직은 Tick_TourExplain에서 첫 라인 출력 시 호출됨
}

void UTourStateSystem::Enter_TourEnd()
{
	PRINTLOG( TEXT("[TourState] Tour End - All waypoints completed"));

	// 모든 투어 웨이포인트 완료
	// 필요 시 추가 처리 (예: 다시 처음부터 시작, 또는 대기 상태)
	if (auto GS = GetWorld()->GetGameState<AYisanGameState>())
	{
		GS->MulticastRPC_ToastMessage(TEXT("모든 투어 일정이 종료되었습니다"));
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

		// 플레이어 수 계산
		auto Players = FComponentHelper::GetAllOfClass<APlayerActor>(GetWorld());
		int32 TotalPlayers = Players.Num();
		int32 NearbyPlayers = 0;
		FVector DasanLocation = OwnerDasan->GetActorLocation();

		for (auto Player : Players)
		{
			float Distance = FVector::Dist(DasanLocation, Player->GetActorLocation());
			if (Distance <= PlayerDetectionRadius)
			{
				NearbyPlayers++;
			}
		}

		if (IsAllPlayersNearby())
		{
			PRINTLOG(TEXT("[TourState] 플레이어 전원 근처 → TourWait"));
			WaitTimer = 0.0f;
			CurState = ETourState::TourWait;

			OwnerDasan->HideExplainDialog();
		}
		else
		{
			PRINTLOG(TEXT("[TourState] 아직 더 모여야 함 (%d/%d)"), NearbyPlayers, TotalPlayers);

			const float DasanGroundSpeed = OwnerDasan->GetGroundSpeed();
			const bool bIsDasanMoving = !FMath::IsNearlyZero(DasanGroundSpeed, 5.0f);

			if (bIsDasanMoving)
			{
				OwnerDasan->HideExplainDialog();
			}
			else
			{
				// ShowExplainDialog로 대기 중인 플레이어 정보 표시
				FString ExplainText = FString::Printf(TEXT("플레이어를 기다리는 중... (%d/%d명)"), NearbyPlayers, TotalPlayers);
				OwnerDasan->ShowExplainDialog(ExplainText);
			}
		}
	}
}

void UTourStateSystem::Tick_TourWait(float DeltaTime)
{
	if (!OwnerDasan)
		return;

	// 강제 집결 타이머 누적
	ForcedGatherTimer += DeltaTime;

	// 플레이어 수 계산
	auto Players = FComponentHelper::GetAllOfClass<APlayerActor>(GetWorld());
	int32 TotalPlayers = Players.Num();
	int32 NearbyPlayers = 0;
	FVector DasanLocation = OwnerDasan->GetActorLocation();

	for (auto Player : Players)
	{
		float Distance = FVector::Dist(DasanLocation, Player->GetActorLocation());
		if (Distance <= PlayerDetectionRadius)
		{
			NearbyPlayers++;
		}
	}

	// 강제 집결 시간이 지나면 모든 플레이어를 텔레포트
	if (ForcedGatherTimer >= ForcedGatherTime)
	{
		PRINTLOG(TEXT("[TourState] 강제 집결 시간 도달 - 모든 플레이어를 다산 위치로 텔레포트"));

		// 다산 앞쪽 방향 계산
		FVector DasanForward = OwnerDasan->GetActorForwardVector();

		// 모든 플레이어를 다산 근처로 텔레포트
		for (auto Player : Players)
		{
			float Distance = FVector::Dist(DasanLocation, Player->GetActorLocation());
			if (Distance > PlayerDetectionRadius)
			{
				// 다산 앞쪽 200 유닛 위치로 텔레포트
				FVector TeleportLocation = DasanLocation + (DasanForward * 200.0f);
				Player->TeleportTo(TeleportLocation, Player->GetActorRotation(), false, true);
				PRINTLOG(TEXT("[TourState] 플레이어 텔레포트: %s"), *Player->GetName());
			}
		}

		// 강제 집결 후 TourMove로 전환
		ForcedGatherTimer = 0.0f;
		CurState = ETourState::TourMove;
		PRINTLOG(TEXT("[TourState] 강제 집결 완료 → TourMove 전환"));

		// TourMove로 전환할 때 실제 이동 명령 실행
		if (OwnerDasan->DasanAicontrol && OwnerDasan->GetCurTargetBuilding())
		{
			FVector TargetPos = OwnerDasan->GetCurTargetBuilding()->GetActorLocation();
			UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

			if (NavSys)
			{
				FNavLocation TargetNavLoc;
				if (NavSys->ProjectPointToNavigation(TargetPos, TargetNavLoc, FVector(5000, 5000, 5000)))
				{
					auto Result = OwnerDasan->AIMoveToLoc( TargetNavLoc.Location, 250.0f, true );
					PRINTLOG(TEXT("[TourState] MoveTo 결과: %d"), (int32)Result.Code);
				}
			}
		}
		return;
	}

	// 주기적 체크 타이머 누적
	WaitTimer += DeltaTime;

	if (WaitTimer >= CheckInterval)
	{
		WaitTimer = 0.0f; // 리셋

		if (IsAllPlayersNearby())
		{
			ForcedGatherTimer = 0.0f; // 타이머 리셋
			CurState = ETourState::TourMove;
			PRINTLOG(TEXT("[TourState] 모든 플레이어 근처 → TourMove 전환"));

			// TourMove로 전환할 때 실제 이동 명령 실행 (NavMesh 위치로)
			if (OwnerDasan->DasanAicontrol && OwnerDasan->GetCurTargetBuilding())
			{
				FVector TargetPos = OwnerDasan->GetCurTargetBuilding()->GetActorLocation();
				UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

				if (NavSys)
				{
					FNavLocation TargetNavLoc;
					if (NavSys->ProjectPointToNavigation(TargetPos, TargetNavLoc, FVector(5000, 5000, 5000)))
					{
						auto Result = OwnerDasan->AIMoveToLoc( TargetNavLoc.Location, 250.0f, true );
						PRINTLOG(TEXT("[TourState] MoveTo 결과: %d"), (int32)Result.Code);
					}
				}
			}
		}
		else
		{
			PRINTLOG(TEXT("[TourState] 일부 플레이어 미도달 (%d/%d)"), NearbyPlayers, TotalPlayers);
		}
	}

	// 강제 집결 카운트다운 표시 (1초마다)
	int32 RemainingGatherTime = FMath::CeilToInt(ForcedGatherTime - ForcedGatherTimer);
	RemainingGatherTime = FMath::Max(0, RemainingGatherTime);

	if (RemainingGatherTime != LastReportedGatherTime)
	{
		// "모두 모일 때까지 기다립니다 (cur/max) - 강제 집결까지 X초"
		FString ExplainText = FString::Printf(TEXT("모두 모일 때까지 기다립니다 (%d/%d명)\n강제 집결까지 %d초"),
			NearbyPlayers, TotalPlayers, RemainingGatherTime);
		OwnerDasan->ShowExplainDialog(ExplainText);

		LastReportedGatherTime = RemainingGatherTime;
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
		// 설명 완료되지 않았다면 설명 라인 출력
		if (!bExplainCompleted)
		{
			// 설명 라인이 있는 경우
			if (ExplainLines.Num() > 0 && ExplainLineIndex < ExplainLines.Num())
			{
				ExplainLineTimer += DeltaTime;

				// 라인 간격이 지났으면 다음 라인 출력
				if (ExplainLineTimer >= ExplainLineInterval)
				{
					// 첫 번째 라인일 때만 음성 재생
					if (ExplainLineIndex == 0)
					{
						StartExplainVoice();
					}

					// 현재 라인 출력
					FString CurrentLine = UCommonFunctionLibrary::RemoveLineBreaks( ExplainLines[ExplainLineIndex]);
					PRINTLOG(TEXT("[TourState] 설명 출력 [%d/%d]: %s"), ExplainLineIndex + 1, ExplainLines.Num(), *CurrentLine);
					
					// 채팅으로 메시지 전송
					OwnerDasan->SendDasanChatMessage(CurrentLine);
					// 머리 위에도 표시
					OwnerDasan->ShowExplainDialog(CurrentLine);

					// 다음 라인으로 이동
					ExplainLineIndex++;
					ExplainLineTimer = 0.0f;
				}
			}
			else
			{
				// 모든 라인 출력 완료
				bExplainCompleted = true;
				PostExplainWaitTimer = 0.0f;
				PRINTLOG(TEXT("[TourState] 모든 설명 라인 출력 완료 - %d초 후 다음 장소로 이동"), (int32)PostExplainWaitDuration);
			}
		}
		else
		{
			// 설명 완료 후 대기
			PostExplainWaitTimer += DeltaTime;

			// 남은 시간 계산
			int32 RemainingTime = FMath::CeilToInt(PostExplainWaitDuration - PostExplainWaitTimer);
			RemainingTime = FMath::Max(0, RemainingTime);

			// 1초 단위로 메시지 전송
			if (RemainingTime != LastReportedTime && RemainingTime >= 0)
			{
				FString ExplainText = FString::Printf(TEXT("다음 장소로 이동까지 %d초"), RemainingTime);
				OwnerDasan->ShowExplainDialog(ExplainText);

				LastReportedTime = RemainingTime;
			}

			// 대기 시간이 끝나면 다음 건물로 이동
			if (PostExplainWaitTimer >= PostExplainWaitDuration)
			{
				PRINTLOG(TEXT("[TourState] 설명 완료 대기 종료 - 다음 건물로 이동"));

				// 애니메이션 끝났다는거 알리기 위함
				if (OwnerDasan)
				{
					OwnerDasan->EndExplainAnim();
					OwnerDasan->HideExplainDialog();
				}

				// 다음 퀘스트(건물)로 이동
				OwnerDasan->NextQuest();

				// 타이머 리셋
				WaitTimer = 0.0f;
				LastReportedTime = -1;
			}
		}
	}
}


void UTourStateSystem::StartExplainVoice()
{
	auto building = OwnerDasan->GetCurTargetBuilding();

	// 건물 에셋 정보 데이터 가져오기
	FBuildingAssetData AssetData;
	if ( UGameDataManager::Get(GetWorld())->GetBuildingAssetData( building->BuildingType, AssetData) )
	{
		if ( auto DetailAsset = AssetData.BuildingDetailDataAsset.LoadSynchronous() )
		{
			TSoftObjectPtr<USoundCue> LoadedCue;
			if ( DetailAsset->LoadSoundCue(LoadedCue) && LoadedCue.Get() )
			{
				// UGameSoundManager를 통해 대화 음성 재생 (기존 음성 자동 중지)
				if (auto SM = UGameSoundManager::Get(GetWorld()))
				{
					SM->PlayConversationVoice(LoadedCue.Get());
				}
			}
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

bool UTourStateSystem::IsUpdateEnble() const
{
	if ( OwnerDasan == nullptr)
		return false;

	if ( OwnerDasan->HasAuthority() == false)
		return false;
	
	return true;
}

static FColor TransparentYellow(255, 255, 0, 64); // (R,G,B,A)

void UTourStateSystem::DrawPlayerRadiusDebug() const
{
	UWorld* World = OwnerDasan->GetWorld();
	if (!World)
		return;

	const FVector Origin = OwnerDasan->GetActorLocation();
	DrawDebugSphere(World, Origin, PlayerDetectionRadius, 8, TransparentYellow, false, 0.25f, 0, 3.0f);
}

