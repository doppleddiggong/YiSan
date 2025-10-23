// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ADasanActor.h"
#include "UTourStateSystem.h"
#include "UExplainStateSystem.h"
#include "UAnswerStateSystem.h"

#include  "AIController.h"

#include "UQuestManager.h"
#include "ABuilding.h"
#include "EBuildingType.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "FComponentHelper.h"
#include "GameLogging.h"
#include "YiSan/YiSan.h"


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

	playerMaxDis = 100.f;
	wayPointDis = 250.f;
	waitChackTimer = 1.f;
}

void ADasanActor::BeginPlay()
{
	Super::BeginPlay();

	// AI 컨트롤러 가져오기
	DasanAicontrol = Cast<AAIController>(GetController());
	if (!DasanAicontrol)
	{
		PRINTLOG(TEXT("AI Controller is not possessed or is not of type AAIController."));
	}

	if (HasAuthority())
	{
		TourStateSystem->InitSystem(this);
		ExplainStateSystem->InitSystem(this);
		AnswerStateSystem->InitSystem(this);
		
		QuestManager = UQuestManager::Get(GetWorld());
		QuestManager->InitSystem();

		// 초기 상태 설정
		DasanState = EDasanState::Tour;
	}
	StartTour();

	GetWorldTimerManager().SetTimer(TourStateTimerHandle, this, &ADasanActor::UpdateTourState, 0.1f, true);
}

void ADasanActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 서버에서만 상태 시스템 틱 실행
	if (HasAuthority())
	{
		switch (DasanState)
		{
		//case EDasanState::Tour:		TourStateSystem->UpdateTick(DeltaTime);
		case EDasanState::Explain:	ExplainStateSystem->UpdateTick(DeltaTime);
		case EDasanState::Answer:	AnswerStateSystem->UpdateTick(DeltaTime);
		default: break;
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
	FString TargetBuildingStr =  QuestManager->GetTargetBuildingName();

	// 상태 정보 조합
	FString StateInfo = FString::Printf(TEXT("[DASAN]\nMain: %s\nTour: %s\nExplain: %s\nAnswer: %s\nTarget: %s"),
		*MainStateStr,
		*TourStateStr,
		*ExplainStateStr,
		*AnswerStateStr,
		*TargetBuildingStr
	);

	// DrawDebugString 호출
	DrawDebugString( GetWorld(),
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
	PRINTLOG( TEXT("DasanActor MainState changed: %s"), *ENUM_TO_NAME(EDasanState, DasanState));
}

// 서버 RPC 구현
void ADasanActor::ServerRPC_SetDasanState_Implementation(EDasanState InState)
{
	DasanState = InState;
}

float ADasanActor::GetTargetBuildingDistnace()
{
	return FVector::Dist( this->GetActorLocation(), this->CurTargetBuilding->GetActorLocation() );
}

void ADasanActor::StartTour()
{
	// 컨트롤러 체크
	if (!DasanAicontrol) 
	{
		PRINTLOG( TEXT("ai 컨트롤러가 존재 하지 않습니다"));
		return;
	}
	
	if (!HasAuthority())
		return;

	DasanState = EDasanState::Tour;

	// TourStateSystem 초기 상태 설정 (움직인다)
	TourStateSystem->SetTourState(ETourState::TourMove);
	
	// 현재 목표 건물 찾기
	CurTargetBuilding = FindCurTargetBuilding();
	PRINTLOG( TEXT("다산 캐릭터의 추적 건물은 : %s 입니다"), *QuestManager->GetTargetBuildingName());

	if (CurTargetBuilding)
	{
		DasanAicontrol->MoveToActor(CurTargetBuilding,wayPointDis);
		PRINTLOG(TEXT("타산 투어 시작합니다 %s "), *CurTargetBuilding->GetName());
	}
	else
	{
		PRINTLOG(TEXT("아 못가 못가 "));
		TourStateSystem->SetTourState(ETourState::TourEnd);
	}
}

void ADasanActor::NextQuest()
{
	// 컨트롤러 체크
	if (!DasanAicontrol) 
	{
		PRINTLOG( TEXT("ai 컨트롤러가 존재 하지 않습니다"));
		return;
	}
	
	if (!HasAuthority())
		return;

	if ( QuestManager->IsHasQuest() )
	{
		TourStateSystem->SetTourState(ETourState::TourMove);
		CurTargetBuilding = FindCurTargetBuilding();
		PRINTLOG( TEXT("DasanActor: Moving to next quest - Target: %s"), *QuestManager->GetTargetBuildingName());
		if (CurTargetBuilding)
		{
			DasanAicontrol->MoveToActor(CurTargetBuilding,wayPointDis);
			PRINTLOG(TEXT("이 지점으로 이동합니다 %s "), *QuestManager->GetTargetBuildingName());
		}
		else
		{
			PRINTLOG(TEXT("지점으로 이동이 불가능합니다"));
			TourStateSystem->SetTourState(ETourState::TourEnd);
		}
	}
	else
	{
		// [TODO], 모든 퀘스트 종료.게임 종료 시퀀스로 갑시다.
		TourStateSystem->SetTourState(ETourState::TourEnd);
		PRINTLOG( TEXT("DasanActor: All quests completed!"));
	}

	
}

// TODO.이함수는 사실 얘가 할거는 아닌데...
ABuilding* ADasanActor::FindCurTargetBuilding() const
{
	if (!QuestManager)
		return nullptr;

	EBuildingType TargetType = QuestManager->GetCurTarget();
	if (TargetType == EBuildingType::None)
		return nullptr;

	// 월드에서 모든 ABuilding 찾기
	auto FoundBuildings = FComponentHelper::GetAllOfClass<ABuilding>(GetWorld());

	// 목표 건물 타입과 일치하는 건물 찾기
	for (auto Building : FoundBuildings)
	{
		if ( Building->BuildingType == TargetType)
		{
			PRINTLOG( TEXT("DasanActor: Found target building - %s"), *Building->GetName());
			return Building;
		}
	}

	PRINTLOG( TEXT("DasanActor: Target building not found for type %s"), *QuestManager->GetTargetBuildingName());
	return nullptr;
}

void ADasanActor::TransitionToState(EDasanState InMainState)
{
	if (!HasAuthority())
	{
		PRINTLOG( TEXT("DasanActor: TransitionToState called on client - ignoring"));
		return;
	}

	if (DasanState == InMainState)
	{
		PRINTLOG( TEXT("DasanActor: Already in state %s"), *ENUM_TO_NAME(EDasanState, InMainState));
		return;
	}

	PRINTLOG( TEXT("DasanActor: Transitioning from %s to %s"),
		*ENUM_TO_NAME(EDasanState, DasanState),
		*ENUM_TO_NAME(EDasanState, InMainState));

	DasanState = InMainState;

	switch (InMainState)
	{
		case EDasanState::Tour: TourStateSystem->SetTourState(ETourState::TourMove);		break;
		case EDasanState::Explain: ExplainStateSystem->SetExplainState(EExplainState::ExplainIng); break;
		case EDasanState::Answer: AnswerStateSystem->SetAnswerState(EAnswerState::AnswerListen);	break;
		default: break;
	}
}


void ADasanActor::UpdateTourState()
{
	if (!TourStateSystem||DasanAicontrol)return;

	ETourState Curstate = TourStateSystem->GetCurState();
	// 플레이어 폰 (dis 에 따른 추적용)
	APawn* player = GetPlayerPawn();
	switch (Curstate)
	{
		case ETourState::TourMove:
			{
				// 건물에 도착했는지 체크
				if (CurTargetBuilding && GetTargetBuildingDistnace() <= wayPointDis)
				{
					PRINTLOG(TEXT("다산님 건물에 도착하십니다"));
					TourStateSystem->SetTourState(ETourState::TourEnd);
					
					// 도착했으니까 설명상태로 변환
					TransitionToState(EDasanState::Explain);
				}
				else if (player && GetPlayerDistance(player) > playerMaxDis)
				{
					TourStateSystem->SetTourState(ETourState::TourWait);
					DasanAicontrol->StopMovement();
					waitChackTimer = 1.f;
				}
				//player 가 너무 멀리 갔다
				break;
			}
		case ETourState::TourWait:
			{
				waitChackTimer = 1.f;
				// 1초 마다 확인
				if (player && GetPlayerDistance(player) <= playerMaxDis)
				{
					PRINTLOG(TEXT("플레이어 가 다시왔"));
					TourStateSystem->SetTourState(ETourState::TourMove);
					if (CurTargetBuilding)
					{
						DasanAicontrol->MoveToActor(CurTargetBuilding,wayPointDis);
					}
					else
					{
						PRINTLOG(TEXT("너 기다리는중 너"));
					}
				}
				break;
				
			}
		case ETourState::TourEnd:
			{
				break;
			}
		default:
			break;
	}	
	
}

float ADasanActor::GetPlayerDistance(class APawn* PlayerPawn) const
{
	if (!PlayerPawn)
	{
		PRINTLOG(TEXT("플레이어 없어요"));
		return 0;
	}
	return FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
}

class APawn* ADasanActor::GetPlayerPawn() const
{
	if (!GetWorld())return nullptr;

	return  UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}




