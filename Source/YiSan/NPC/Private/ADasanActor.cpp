// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#include "ADasanActor.h"
#include "UTourStateSystem.h"
#include "UExplainStateSystem.h"
#include "UAnswerStateSystem.h"

#include "AIController.h"

#include "UQuestManager.h"
#include "ABuilding.h"
#include "EBuildingType.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "FComponentHelper.h"
#include "GameLogging.h"
#include "YiSan/YiSan.h"
#include "GameFramework/CharacterMovementComponent.h"


ADasanActor::ADasanActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 네트워크 복제 활성화
	bReplicates = true;
	SetReplicateMovement(true);

	// 상태 시스템 컴포넌트 생성
	TourStateSystem = CreateDefaultSubobject<UTourStateSystem>(TEXT("TourStateSystem"));
	ExplainStateSystem = CreateDefaultSubobject<UExplainStateSystem>(TEXT("ExplainStateSystem"));
	AnswerStateSystem = CreateDefaultSubobject<UAnswerStateSystem>(TEXT("AnswerStateSystem"));

	playerMaxDis = 1000.f;
	wayPointDis = 250.f;
	waitChackTimer = 1.f;
}

void ADasanActor::BeginPlay()
{
	Super::BeginPlay();

	// 값 설정
	playerMaxDis = 1000.f;
	wayPointDis = 250.f;
	waitChackTimer = 1.f;
	
	PRINTLOG(TEXT("========== ADasanActor BeginPlay =========="));
	PRINTLOG(TEXT(" playerMaxDis: %.1f"), playerMaxDis);
	PRINTLOG(TEXT(" wayPointDis: %.1f"), wayPointDis);

	if (HasAuthority())
	{
		// AI 컨트롤러 가져오기
		DasanAicontrol = Cast<AAIController>(GetController());
		if (!DasanAicontrol)
		{
			PRINTLOG(TEXT(" [CRITICAL] AI Controller is not possessed!"));
			PRINTLOG(TEXT(" 블루프린트에서 'Auto Possess AI'를 확인하세요."));
			PRINTLOG(TEXT(" 현재 Controller: %s"), GetController() ? *GetController()->GetName() : TEXT("nullptr"));
			// AI Controller 없어도 일단 진행 (직접 이동 방식 사용)
		}
		else
		{
			PRINTLOG(TEXT(" AI Controller 초기화 성공: %s"), *DasanAicontrol->GetName());
		}

		// 시스템 초기화
		TourStateSystem->InitSystem(this);
		ExplainStateSystem->InitSystem(this);
		AnswerStateSystem->InitSystem(this);
		
		QuestManager = UQuestManager::Get(GetWorld());
		if (QuestManager)
		{
			QuestManager->InitSystem();
			PRINTLOG(TEXT(" QuestManager 초기화 성공"));
		}
		else
		{
			PRINTLOG(TEXT(" [CRITICAL] QuestManager 초기화 실패!"));
		}

		// 초기 상태 설정
		DasanState = EDasanState::Tour;
		
		// 투어 시작
		PRINTLOG(TEXT(" StartTour 호출 예정"));
		StartTour();
	}

	// 타이머 시작 (Authority만)
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(TourStateTimerHandle, this, &ADasanActor::UpdateTourState, 0.1f, true);
		PRINTLOG(TEXT(" TourState 타이머 시작"));
	}
}

void ADasanActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 서버에서만 상태 시스템 틱 실행
	if (HasAuthority())
	{
		switch (DasanState)
		{
		case EDasanState::Explain:	
			ExplainStateSystem->UpdateTick(DeltaTime);
			break;
		case EDasanState::Answer:	
			AnswerStateSystem->UpdateTick(DeltaTime);
			break;
		case EDasanState::Tour:
		default: 
			break;
		}
	}

	DrawDebugState();
}

void ADasanActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADasanActor, DasanState);
}



void ADasanActor::DrawDebugState()
{
	if (!GetWorld())
		return;

	// 캐릭터 머리 위 위치 계산
	FVector DrawLocation = GetActorLocation() + FVector(0, 0, 120.0f);

	FString MainStateStr = *ENUM_TO_NAME(EDasanState, DasanState);
	FString TourStateStr = *ENUM_TO_NAME(ETourState, TourStateSystem->GetCurState());
	FString ExplainStateStr = *ENUM_TO_NAME(EExplainState, ExplainStateSystem->GetCurState());
	FString AnswerStateStr = *ENUM_TO_NAME(EAnswerState, AnswerStateSystem->GetCurState());
	FString TargetBuildingStr = QuestManager != nullptr ? QuestManager->GetTargetBuildingName() : TEXT("Unknown");
	FString AIControllerStr = DasanAicontrol ? TEXT("OK") : TEXT("NULL");

	// 상태 정보 조합
	FString StateInfo = FString::Printf(TEXT("[DASAN]\nMain: %s\nTour: %s\nExplain: %s\nAnswer: %s\nTarget: %s\nAI: %s"),
		*MainStateStr,
		*TourStateStr,
		*ExplainStateStr,
		*AnswerStateStr,
		*TargetBuildingStr,
		*AIControllerStr
	);

	// DrawDebugString 호출
	DrawDebugString(GetWorld(),
		DrawLocation,
		StateInfo,
		nullptr,
		FColor::Cyan,
		0.0f,
		true
	);
}

// RepNotify 함수
void ADasanActor::OnRep_DasanState()
{
	PRINTLOG(TEXT("DasanActor MainState changed: %s"), *ENUM_TO_NAME(EDasanState, DasanState));
}

// 서버 RPC 구현
void ADasanActor::ServerRPC_SetDasanState_Implementation(EDasanState InState)
{
	DasanState = InState;
}

float ADasanActor::GetTargetBuildingDistnace()
{
	if (!CurTargetBuilding)
	{
		PRINTLOG(TEXT(" CurTargetBuilding이 nullptr임!"));
		return -1.0f;
	}
	return FVector::Dist(this->GetActorLocation(), this->CurTargetBuilding->GetActorLocation());
}

void ADasanActor::StartTour()
{
	PRINTLOG(TEXT("========== StartTour 호출 =========="));

	if (!HasAuthority())
	{
		PRINTLOG(TEXT("️ StartTour: 클라이언트에서 호출됨 (무시)"));
		return;
	}
	
	// 첫 번째 목표 건물 찾기
	CurTargetBuilding = FindCurTargetBuilding();
	
	if (QuestManager)
	{
		PRINTLOG(TEXT(" 다산 캐릭터의 추적 건물: %s"), *QuestManager->GetTargetBuildingName());
	}
	else
	{
		PRINTLOG(TEXT(" [CRITICAL] QuestManager가 nullptr!"));
		return;
	}

	if (CurTargetBuilding)
	{
		float Distance = GetTargetBuildingDistnace();
		PRINTLOG(TEXT(" 타겟 건물 발견: %s (거리: %.1f)"), 
			*CurTargetBuilding->GetName(), 
			Distance);
		
		// 상태를 먼저 설정
		DasanState = EDasanState::Tour;
		
		// 투어 시작 시 플레이어 거리 체크
		APawn* Player = GetPlayerPawn();
		if (Player && GetPlayerDistance(Player) > playerMaxDis)
		{
			PRINTLOG(TEXT(" 플레이어가 너무 멀리 있음. 대기 상태(TourWait)에서 시작함"));
			TourStateSystem->SetTourState(ETourState::TourWait);
			waitChackTimer = 1.f;
		}
		else
		{
			PRINTLOG(TEXT(" 플레이어가 근처에 있음. 이동 상태(TourMove) 시작함"));
			TourStateSystem->SetTourState(ETourState::TourMove);
			
			// AI Controller로 이동 시작
			if (DasanAicontrol)
			{
				FAIMoveRequest MoveRequest;
				MoveRequest.SetGoalActor(CurTargetBuilding);
				MoveRequest.SetAcceptanceRadius(250.0f);
				
				FPathFollowingRequestResult Result = DasanAicontrol->MoveTo(MoveRequest);
				if (Result.Code == EPathFollowingRequestResult::RequestSuccessful)
				{
					PRINTLOG(TEXT(" AI MoveTo 시작 성공"));
				}
				else
				{
					PRINTLOG(TEXT(" AI MoveTo 실패: %d"), (int32)Result.Code);
				}
			}
			else
			{
				PRINTLOG(TEXT(" AI Controller가 없음 - 직접 이동 모드"));
			}
		}
		
		PRINTLOG(TEXT(" StartTour 완료 - DasanState: %s, TourState: %s"), 
			*ENUM_TO_NAME(EDasanState, DasanState),
			*ENUM_TO_NAME(ETourState, TourStateSystem->GetCurState()));
	}
	else
	{
		PRINTLOG(TEXT(" [CRITICAL] 타겟 건물을 찾을 수 없음!"));
		DasanState = EDasanState::Tour;
		TourStateSystem->SetTourState(ETourState::TourEnd);
	}
}

ABuilding* ADasanActor::FindCurTargetBuilding() const
{
	if (!QuestManager)
	{
		PRINTLOG(TEXT(" QuestManager가 nullptr임!"));
		return nullptr;
	}

	EBuildingType TargetType = QuestManager->GetCurTarget();
	if (TargetType == EBuildingType::None)
	{
		PRINTLOG(TEXT(" 타겟 건물 타입이 None임"));
		return nullptr;
	}

	// 월드에서 모든 ABuilding 찾기
	auto FoundBuildings = FComponentHelper::GetAllOfClass<ABuilding>(GetWorld());
	PRINTLOG(TEXT(" 월드에서 건물 검색 중... (총 %d개 발견)"), FoundBuildings.Num());

	// 목표 건물 타입과 일치하는 건물 찾기
	for (auto Building : FoundBuildings)
	{
		if (Building && Building->BuildingType == TargetType)
		{
			PRINTLOG(TEXT(" 타겟 건물 발견: %s (타입 일치)"), *Building->GetName());
			return Building;
		}
	}

	PRINTLOG(TEXT(" 타겟 건물을 찾을 수 없음: %s"), *QuestManager->GetTargetBuildingName());
	return nullptr;
}

void ADasanActor::TransitionToState(EDasanState InMainState)
{
	if (!HasAuthority())
	{
		PRINTLOG(TEXT(" TransitionToState: 클라이언트에서 호출됨 (무시)"));
		return;
	}

	if (DasanState == InMainState)
	{
		PRINTLOG(TEXT(" TransitionToState: 이미 %s 상태임"), *ENUM_TO_NAME(EDasanState, InMainState));
		return;
	}

	PRINTLOG(TEXT(" 상태 전환: %s → %s"),
		*ENUM_TO_NAME(EDasanState, DasanState),
		*ENUM_TO_NAME(EDasanState, InMainState));

	DasanState = InMainState;

	switch (InMainState)
	{
	case EDasanState::Tour:
		PRINTLOG(TEXT(" Tour 상태 시작 - 목표: %s"), CurTargetBuilding ? *CurTargetBuilding->GetName() : TEXT("None"));
		TourStateSystem->SetTourState(ETourState::TourMove);
		
		// AI Controller로 이동 시작
		if (DasanAicontrol && CurTargetBuilding)
		{
			FAIMoveRequest MoveRequest;
			MoveRequest.SetGoalActor(CurTargetBuilding);
			MoveRequest.SetAcceptanceRadius(250.0f);
			DasanAicontrol->MoveTo(MoveRequest);
			PRINTLOG(TEXT(" AI MoveTo 시작"));
		}
		break;
		
	case EDasanState::Explain:
		PRINTLOG(TEXT(" Explain 상태 시작"));
		ExplainStateSystem->SetExplainState(EExplainState::ExplainIng);
		break;
		
	case EDasanState::Answer:
		PRINTLOG(TEXT(" Answer 상태 시작"));
		AnswerStateSystem->SetAnswerState(EAnswerState::AnswerListen);
		break;
		
	default:
		break;
	}
}

void ADasanActor::UpdateTourState()
{
	// 메인상태가 투어가 아니라면 함수를 실행 하지 않는다
	if(DasanState != EDasanState::Tour)
		return;
	
	if (!TourStateSystem || !DasanAicontrol)
	{
		// 매 프레임 로그 방지: static 변수로 1회만 출력
		static bool bLoggedOnce = false;
		if (!bLoggedOnce)
		{
			if (!TourStateSystem)
				PRINTLOG(TEXT(" TourStateSystem이 nullptr입니다!"));
			if (!DasanAicontrol)
				PRINTLOG(TEXT(" DasanAicontrol이 nullptr입니다!"));
			bLoggedOnce = true;
		}
		return;
	}

	ETourState Curstate = TourStateSystem->GetCurState();
	APawn* player = GetPlayerPawn();

	switch (Curstate)
	{
	case ETourState::TourMove:
		{
			// 건물에 도착했는지 체크
			float Distance = GetTargetBuildingDistnace();
			if (CurTargetBuilding && Distance > 0 && Distance <= 500.f) // 250에서 500으로 변경
			{
				PRINTLOG(TEXT(" 다산님 건물 도착! (거리: %.1f)"), Distance);
				TourStateSystem->SetTourState(ETourState::TourEnd);

				// 도착했으니까 설명 상태로 변환
				TransitionToState(EDasanState::Explain);
			}
			else if (player && GetPlayerDistance(player) > playerMaxDis)
			{
				PRINTLOG(TEXT(" 플레이어가 너무 멀어짐 (거리: %.1f) - 대기 상태"), GetPlayerDistance(player));
				TourStateSystem->SetTourState(ETourState::TourWait);
				DasanAicontrol->StopMovement();
				waitChackTimer = 1.f;
			}
			break;
		}
	case ETourState::TourWait:
		{
			waitChackTimer -= 0.1f; // 타이머 감소
		
			// 1초마다 확인
			if (waitChackTimer <= 0.0f)
			{
				waitChackTimer = 1.f;
			
				if (player && GetPlayerDistance(player) <= playerMaxDis)
				{
					PRINTLOG(TEXT(" 플레이어 복귀 - 다시 이동"));
					TourStateSystem->SetTourState(ETourState::TourMove);
				
					if (CurTargetBuilding)
					{
						DasanAicontrol->MoveToActor(CurTargetBuilding, wayPointDis);
					}
					else
					{
						PRINTLOG(TEXT("CurTargetBuilding이 nullptr!"));
					}
				}
				else
				{
					// 플레이어가 아직 멀리 있다면
					PRINTLOG(TEXT(" 플레이어 대기 중..."));
				}
			}
			break;
		}
	case ETourState::TourEnd:
		{
			// 종료 상태
			break;
		}
	default:
		break;
	}
}

void ADasanActor::NextQuest()
{
	// 컨트롤러 체크
	if (!DasanAicontrol)
	{
		PRINTLOG(TEXT(" AI 컨트롤러가 존재하지 않습니다"));
		return;
	}

	if (!HasAuthority())
		return;

	if (QuestManager && QuestManager->IsHasQuest())
	{
		// 다음 퀘스트 시작 시 player 의 거리를 체크함
		CurTargetBuilding = FindCurTargetBuilding();
		PRINTLOG(TEXT("DasanActor: Moving to next quest - Target: %s"), *QuestManager->GetTargetBuildingName());
		
		if (CurTargetBuilding)
		{
			PRINTLOG(TEXT(" 다음 목표 건물까지 거리: %.1f"), GetTargetBuildingDistnace());
			
			APawn* Player = GetPlayerPawn();
			if (Player && GetPlayerDistance(Player) > playerMaxDis)
			{
				PRINTLOG(TEXT(" 플레이어가 너무 멀리 있음. 대기 상태 시작함"));
				TourStateSystem->SetTourState(ETourState::TourWait);
				waitChackTimer = 1.f;
			}
			else
			{
				PRINTLOG(TEXT(" 플레이어가 근처에 있음. 이동 상태 시작함"));
				TourStateSystem->SetTourState(ETourState::TourMove);
				DasanAicontrol->MoveToActor(CurTargetBuilding, wayPointDis);
				PRINTLOG(TEXT(" 다음 목적지로 이동: %s"), *QuestManager->GetTargetBuildingName());
			}
		}
		else
		{
			PRINTLOG(TEXT(" 목적지 이동 불가"));
			TourStateSystem->SetTourState(ETourState::TourEnd);
		}
	}
	else
	{
		// 모든 퀘스트 종료
		TourStateSystem->SetTourState(ETourState::TourEnd);
		PRINTLOG(TEXT(" DasanActor: All quests completed!"));
	}
}

float ADasanActor::GetPlayerDistance(class APawn* PlayerPawn) const
{
	if (!PlayerPawn)
	{
		return -1.0f;
	}
	return FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
}

class APawn* ADasanActor::GetPlayerPawn() const
{
	if (!GetWorld())
		return nullptr;

	return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}