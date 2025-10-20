// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ADasanActor.h"
#include "UTourStateSystem.h"
#include "UExplainStateSystem.h"
#include "UAnswerStateSystem.h"

#include "UQuestManager.h"
#include "ABuilding.h"
#include "EBuildingType.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "FComponentHelper.h"
#include "GameLogging.h"

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

}

void ADasanActor::BeginPlay()
{
	Super::BeginPlay();

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
}

void ADasanActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 서버에서만 상태 시스템 틱 실행
	if (HasAuthority())
	{
		switch (DasanState)
		{
		case EDasanState::Tour:		TourStateSystem->UpdateTick(DeltaTime);
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
	if (!HasAuthority())
		return;

	DasanState = EDasanState::Tour;

	// TourStateSystem 초기 상태 설정
	TourStateSystem->SetTourState(ETourState::TourMove);

	// 현재 목표 건물 찾기
	CurTargetBuilding = FindCurTargetBuilding();
	PRINTLOG( TEXT("Dasan Tour Started! Target: %s"), *QuestManager->GetTargetBuildingName());
}

void ADasanActor::NextQuest()
{
	if (!HasAuthority())
		return;

	if ( QuestManager->IsHasQuest() )
	{
		TourStateSystem->SetTourState(ETourState::TourMove);
		CurTargetBuilding = FindCurTargetBuilding();
		PRINTLOG( TEXT("DasanActor: Moving to next quest - Target: %s"), *QuestManager->GetTargetBuildingName());
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