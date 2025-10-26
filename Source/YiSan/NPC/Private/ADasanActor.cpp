// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#include "ADasanActor.h"
#include "UTourStateSystem.h"
#include "UAnswerStateSystem.h"

#include "AIController.h"

#include "UQuestManager.h"
#include "ABuilding.h"
#include "APlayerActor.h"
#include "EBuildingType.h"
#include "UBroadcastManager.h"
#include "EVoiceCommandType.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "FComponentHelper.h"
#include "GameLogging.h"

#include "Components/WidgetComponent.h"
#include "UDasanWidget.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UChatPlayerSystem.h"
#include "UGameSoundManager.h"
#include "YiSan/YiSan.h"

#define DASANWIDGET_PATH TEXT("/Game/CustomContents/UI/WBP_DasanWidget.WBP_DasanWidget_C")

ADasanActor::ADasanActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 네트워크 복제 활성화
	bReplicates = true;
	SetReplicateMovement(true);

	// 상태 시스템 컴포넌트 생성
	TourStateSystem = CreateDefaultSubobject<UTourStateSystem>(TEXT("TourStateSystem"));
	AnswerStateSystem = CreateDefaultSubobject<UAnswerStateSystem>(TEXT("AnswerStateSystem"));

	// 위젯 컴포넌트 생성 (머리 위에 표시)
	DasanWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("DasanWidgetComponent"));
	DasanWidgetComp->SetupAttachment(RootComponent);
	DasanWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 170.f));  // 머리 위 Z+170
	DasanWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	DasanWidgetComp->SetDrawSize(FVector2D(300.f, 100.f));

	// 위젯 클래스 설정
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClassFinder(DASANWIDGET_PATH);
	if (WidgetClassFinder.Succeeded())
		DasanWidgetComp->SetWidgetClass(WidgetClassFinder.Class);

	playerMaxDis = 1000.f;
	wayPointDis = 250.f;
	waitChackTimer = 1.f;
	
	// 초기 상태 (None에서 시작)
	DasanState = EDasanState::None;
}

void ADasanActor::BeginPlay()
{
	Super::BeginPlay();

	if (!TourStateSystem)
	{
		PRINTLOG(TEXT("TourStateSystem::nullptr이라 재생성합니다."));
		TourStateSystem = NewObject<UTourStateSystem>(this, UTourStateSystem::StaticClass());
		TourStateSystem->RegisterComponent();
	}
	
	if (!AnswerStateSystem)
	{
		PRINTLOG(TEXT("AnswerStateSystem::nullptr이라 재생성합니다."));
		AnswerStateSystem = NewObject<UAnswerStateSystem>(this, UAnswerStateSystem::StaticClass());
		AnswerStateSystem->RegisterComponent();
	}

	// 위젯 캐싱 및 초기화
	if (DasanWidgetComp && DasanWidgetComp->GetWidget())
	{
		DasanWidget = Cast<UDasanWidget>(DasanWidgetComp->GetWidget());
		DasanWidget->InitWidget(this);
	}

	// 값 설정
	playerMaxDis = 1000.f;
	wayPointDis = 250.f;
	waitChackTimer = 1.f;
	
	PRINTLOG(TEXT("========== ADasanActor BeginPlay =========="));
	PRINTLOG(TEXT("[Dasan] playerMaxDis: %.1f"), playerMaxDis);
	PRINTLOG(TEXT("[Dasan] wayPointDis: %.1f"), wayPointDis);

	if (HasAuthority())
	{
		// AI 컨트롤러 가져오기
		DasanAicontrol = Cast<AAIController>(GetController());
		if (!DasanAicontrol)
		{
			PRINTLOG(TEXT("[Dasan] 블루프린트에서 'Auto Possess AI'를 확인하세요."));
			PRINTLOG(TEXT("[Dasan] 현재 Controller: %s"), GetController() ? *GetController()->GetName() : TEXT("nullptr"));
		}
		else
		{
			PRINTLOG(TEXT("[Dasan] AI Controller 초기화 성공: %s"), *DasanAicontrol->GetName());
			
			// AI MoveTo 완료 콜백 바인딩
			DasanAicontrol->ReceiveMoveCompleted.AddDynamic(this, &ADasanActor::OnMoveCompleted);
		}

		// 시스템 초기화
		TourStateSystem->InitSystem(this);
		AnswerStateSystem->InitSystem(this);
		
		QuestManager = UQuestManager::Get(GetWorld());
		if (QuestManager)
		{
			QuestManager->InitSystem();
			PRINTLOG(TEXT("[Dasan] QuestManager 초기화 성공"));
		}
		
		// 투어 시작 (TransitionToState를 통해 초기화)
		StartTour();
	}

	// 타이머 시작 (Authority만)
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(TourStateTimerHandle, this, &ADasanActor::UpdateTourState, 0.1f, true);
		PRINTLOG(TEXT("[Dasan] TourState 타이머 시작"));
	}

	// BroadcastManager 이벤트 구독
	BroadcastManager = UBroadcastManager::Get(GetWorld());
	if (BroadcastManager)
	{
		BroadcastManager->OnExecVoiceCommand.AddDynamic(this, &ADasanActor::OnExecVoiceCommand);
		PRINTLOG(TEXT("[Dasan] BroadcastManager 이벤트 구독 성공"));
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
		case EDasanState::Answer: 
			AnswerStateSystem->UpdateTick(DeltaTime); 
			break;
		case EDasanState::Tour:
		default: 
			break;
		}
	}
}

void ADasanActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADasanActor, DasanState);
}

// AI MoveTo 완료 콜백
void ADasanActor::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	if (!HasAuthority())
		return;

	switch (Result)
	{
	case EPathFollowingResult::Success:
		PRINTLOG(TEXT("[Dasan] AI MoveTo 성공 - 목적지 도착"));

		// Tour 상태일 때만 처리
		if (DasanState == EDasanState::Tour && TourStateSystem)
		{
			// TourEnd 상태로 전환
			TourStateSystem->SetTourState(ETourState::TourEnd);
			
			// 다음 퀘스트 확인 및 이동
			if (QuestManager && QuestManager->IsHasQuest() && QuestManager->GetCurTarget() != EBuildingType::None)
			{
				PRINTLOG(TEXT("[Dasan] 다음 퀘스트로 이동"));
				
				// 짧은 대기 후 다음 퀘스트로 이동 (0.5초)
				FTimerHandle NextQuestTimer;
				GetWorldTimerManager().SetTimer(NextQuestTimer, [this]()
				{
					NextQuest();
				}, 0.5f, false);
			}
			else
			{
				// 모든 퀘스트 완료 - Answer 상태로 전환
				PRINTLOG(TEXT("[Dasan] 모든 퀘스트 완료 - Answer 상태로 전환"));
				TransitionToState(EDasanState::Answer);
			}
		}
		break;

	case EPathFollowingResult::Blocked:
		PRINTLOG(TEXT("[Dasan] AI MoveTo 차단됨 - 재시도"));
		// 0.5초 후 재시도
		if (CurTargetBuilding && DasanAicontrol)
		{
			FTimerHandle RetryTimer;
			GetWorldTimerManager().SetTimer(RetryTimer, [this]()
			{
				if (CurTargetBuilding && DasanAicontrol)
				{
					FAIMoveRequest MoveRequest;
					MoveRequest.SetGoalActor(CurTargetBuilding);
					MoveRequest.SetAcceptanceRadius(wayPointDis);
					MoveRequest.SetUsePathfinding(true);
					DasanAicontrol->MoveTo(MoveRequest);
					PRINTLOG(TEXT("[Dasan] MoveTo 재시도"));
				}
			}, 0.5f, false);
		}
		break;

	case EPathFollowingResult::Aborted:
		PRINTLOG(TEXT("[Dasan] AI MoveTo 중단됨"));
		break;

	case EPathFollowingResult::Invalid:
		PRINTLOG(TEXT("[Dasan] AI MoveTo 실패 - 경로 없음 (NavMesh 확인 필요)"));
		
		// 경로를 찾지 못한 경우 - 직접 이동 모드로 폴백
		if (TourStateSystem && CurTargetBuilding)
		{
			PRINTLOG(TEXT("[Dasan] ========== 직접 이동 모드 활성화 =========="));
			PRINTLOG(TEXT("[Dasan] 목표: %s"), *CurTargetBuilding->GetName());
			PRINTLOG(TEXT("[Dasan] 현재 상태: %s"), *ENUM_TO_NAME(ETourState, TourStateSystem->GetCurState()));
			
			// TourEnd 대신 TourMove 유지하고 직접 이동
			TourStateSystem->SetTourState(ETourState::TourMove);
			
			// UpdateTourState에서 직접 이동 처리
			bUseDirectMovement = true;
			
			PRINTLOG(TEXT("[Dasan] bUseDirectMovement = TRUE 설정 완료"));
		}
		break;

	default:
		break;
	}
}

void ADasanActor::OnRep_DasanState()
{
	PRINTLOG(TEXT("[Dasan] OnRep_DasanState: %s"), *ENUM_TO_NAME(EDasanState, DasanState));
	UpdateWidgetState();
}

void ADasanActor::ServerRPC_SetDasanState_Implementation(EDasanState InState)
{
	if (DasanState == InState)
		return;

	PRINTLOG(TEXT("[Dasan] ServerRPC_SetDasanState: %s -> %s"),
		*ENUM_TO_NAME(EDasanState, DasanState),
		*ENUM_TO_NAME(EDasanState, InState));

	DasanState = InState;
	OnRep_DasanState();
}

ABuilding* ADasanActor::FindCurTargetBuilding() const
{
	if (!QuestManager)
	{
		PRINTLOG(TEXT("[Dasan] FindCurTargetBuilding: QuestManager가 nullptr"));
		return nullptr;
	}

	EBuildingType TargetType = QuestManager->GetCurTarget();
	if (TargetType == EBuildingType::None)
	{
		PRINTLOG(TEXT("[Dasan] FindCurTargetBuilding: TargetType이 None"));
		return nullptr;
	}

	// 월드에서 모든 ABuilding 찾기
	TArray<AActor*> Buildings;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuilding::StaticClass(), Buildings);

	for (AActor* Actor : Buildings)
	{
		ABuilding* Building = Cast<ABuilding>(Actor);
		if (Building && Building->BuildingType == TargetType)
		{
			PRINTLOG(TEXT("[Dasan] 목표 건물 찾음: %s"), *Building->GetName());
			return Building;
		}
	}

	PRINTLOG(TEXT("[Dasan] 목표 건물을 찾지 못함: %s"), *ENUM_TO_NAME(EBuildingType, TargetType));
	return nullptr;
}

float ADasanActor::GetTargetBuildingDistnace()
{
	if (!CurTargetBuilding)
		return -1.0f;

	return FVector::Dist(GetActorLocation(), CurTargetBuilding->GetActorLocation());
}

void ADasanActor::StartTour()
{
	if (!HasAuthority())
		return;

	PRINTLOG(TEXT("[Dasan] ========== 투어 시작 =========="));

	// 첫 번째 목표 건물 찾기
	CurTargetBuilding = FindCurTargetBuilding();

	if (!CurTargetBuilding)
	{
		PRINTLOG(TEXT("[Dasan] 목표 건물을 찾을 수 없습니다."));
		return;
	}

	PRINTLOG(TEXT("[Dasan] 첫 번째 목표: %s"), *CurTargetBuilding->GetName());

	// Tour 상태로 전환
	TransitionToState(EDasanState::Tour);
}

void ADasanActor::TransitionToState(EDasanState InMainState)
{
	if (!HasAuthority())
		return;

	if (DasanState == InMainState)
	{
		PRINTLOG(TEXT("[Dasan] 이미 %s 상태입니다."), *ENUM_TO_NAME(EDasanState, InMainState));
		return;
	}

	PRINTLOG(TEXT("[Dasan] 상태 전환: %s -> %s"),
		*ENUM_TO_NAME(EDasanState, DasanState),
		*ENUM_TO_NAME(EDasanState, InMainState));

	DasanState = InMainState;

	switch (InMainState)
	{
	case EDasanState::Tour:
	{
		PRINTLOG(TEXT("[Dasan] Tour 상태 시작"));
		
		if (!CurTargetBuilding)
		{
			CurTargetBuilding = FindCurTargetBuilding();
		}

		if (!CurTargetBuilding)
		{
			PRINTLOG(TEXT("[Dasan] 목표 건물이 없습니다. Tour 종료"));
			if (TourStateSystem)
			{
				TourStateSystem->SetTourState(ETourState::TourEnd);
			}
			return;
		}

		const FString TargetName = CurTargetBuilding->GetName();
		PRINTLOG(TEXT("[Dasan] Tour 상태 시작 - 목표: %s"), *TargetName);

		if (TourStateSystem)
		{
			TourStateSystem->SetTourState(ETourState::TourMove);
		}

		// AI Controller로 이동 시작
		if (DasanAicontrol && CurTargetBuilding)
		{
			FAIMoveRequest MoveRequest;
			MoveRequest.SetGoalActor(CurTargetBuilding);
			MoveRequest.SetAcceptanceRadius(wayPointDis);
			MoveRequest.SetUsePathfinding(true);

			FPathFollowingRequestResult MoveResult = DasanAicontrol->MoveTo(MoveRequest);
			
			if (MoveResult.Code == EPathFollowingRequestResult::RequestSuccessful)
			{
				PRINTLOG(TEXT("[Dasan] AI MoveTo 시작 성공"));
			}
			else
			{
				PRINTLOG(TEXT("[Dasan] AI MoveTo 실패 - 코드: %d (NavMesh 확인 필요)"), (int32)MoveResult.Code);
			}
		}
		else
		{
			PRINTLOG(TEXT("[Dasan] AI Controller 또는 Target이 없어 이동 불가"));
		}
	}
	break;

	case EDasanState::Answer:
	{
		PRINTLOG(TEXT("[Dasan] Answer 상태 시작"));
		
		// 이동 중지
		if (DasanAicontrol)
		{
			DasanAicontrol->StopMovement();
		}
		
		if (AnswerStateSystem)
		{
			AnswerStateSystem->SetAnswerState(EAnswerState::AnswerListen);
		}
	}
	break;

	default:
		break;
	}

	// 위젯 업데이트
	UpdateWidgetState();
}

void ADasanActor::UpdateTourState()
{
	// 메인상태가 투어가 아니라면 실행하지 않음
	if (DasanState != EDasanState::Tour)
	{
		PRINTLOG(TEXT("[Dasan][UpdateTourState] DasanState가 Tour가 아님: %s"), *ENUM_TO_NAME(EDasanState, DasanState));
		return;
	}
	
	if (!TourStateSystem || !DasanAicontrol)
	{
		PRINTLOG(TEXT("[Dasan][UpdateTourState] TourStateSystem 또는 AIController 없음"));
		return;
	}

	ETourState CurState = TourStateSystem->GetCurState();
	APawn* Player = GetPlayerPawn();
	
	// 디버그: 현재 상태 출력 (30프레임마다)
	static int32 StateCheckCounter = 0;
	if (++StateCheckCounter % 30 == 0)
	{
		PRINTLOG(TEXT("[Dasan][UpdateTourState] CurState: %s, bUseDirectMovement: %s"), 
			*ENUM_TO_NAME(ETourState, CurState),
			bUseDirectMovement ? TEXT("TRUE") : TEXT("FALSE"));
	}

	switch (CurState)
	{
	case ETourState::TourMove:
	{
		// 직접 이동 모드인 경우
		if (bUseDirectMovement && CurTargetBuilding)
		{
			PRINTLOG(TEXT("[Dasan][DirectMove] === 직접 이동 실행 중 ==="));
			
			// 목표 방향 계산
			FVector CurLoc = GetActorLocation();
			FVector TargetLoc = CurTargetBuilding->GetActorLocation();
			
			// 높이 차이 체크 (안전장치)
			float HeightDiff = FMath::Abs(CurLoc.Z - TargetLoc.Z);
			if (HeightDiff > 500.0f)  // 500 유닛 이상 높이 차이
			{
				PRINTLOG(TEXT(" 높이 차이가 너무 큼 (%.1f) - NavMesh 필요!"), HeightDiff);
				PRINTLOG(TEXT(" 직접 이동 실패 - 다음 퀘스트로 스킵"));
				
				bUseDirectMovement = false;
				TourStateSystem->SetTourState(ETourState::TourEnd);
				
				// 다음 퀘스트
				FTimerHandle NextQuestTimer;
				GetWorldTimerManager().SetTimer(NextQuestTimer, [this]()
				{
					NextQuest();
				}, 0.5f, false);
				return;
			}
			
			FVector Direction = (TargetLoc - CurLoc).GetSafeNormal();
			float Distance = FVector::Dist(CurLoc, TargetLoc);

			// 디버그 로그 (10프레임마다)
			static int32 FrameCounter = 0;
			if (++FrameCounter % 10 == 0)
			{
				PRINTLOG(TEXT("[Dasan][DirectMove] 이동 중... 거리: %.1f / %.1f, 높이차: %.1f"), 
					Distance, wayPointDis, HeightDiff);
			}

			// 회전
			FRotator TargetRotation = Direction.Rotation();
			SetActorRotation(FMath::RInterpTo(
				GetActorRotation(),
				TargetRotation,
				GetWorld()->GetDeltaSeconds(),
				5.0f
			));

			// 도착 체크
			if (Distance <= wayPointDis)
			{
				PRINTLOG(TEXT("[Dasan][DirectMove] ========== 목표 도착! =========="));
				bUseDirectMovement = false;
				FrameCounter = 0;
				
				// 도착 처리
				TourStateSystem->SetTourState(ETourState::TourEnd);
				
				// OnMoveCompleted의 Success 로직 실행
				if (QuestManager && QuestManager->IsHasQuest() && 
					QuestManager->GetCurTarget() != EBuildingType::None)
				{
					PRINTLOG(TEXT("[Dasan] 다음 퀘스트로 이동"));
					
					FTimerHandle NextQuestTimer;
					GetWorldTimerManager().SetTimer(NextQuestTimer, [this]()
					{
						NextQuest();
					}, 0.5f, false);
				}
				else
				{
					PRINTLOG(TEXT("[Dasan] 모든 퀘스트 완료 - Answer 상태로 전환"));
					TransitionToState(EDasanState::Answer);
				}
				return;
			}

			// 이동 - 수평 방향만 (Z축 무시)
			UCharacterMovementComponent* Movement = GetCharacterMovement();
			if (Movement)
			{
				// 수평 방향만 계산 (Z=0)
				FVector HorizontalDirection = Direction;
				HorizontalDirection.Z = 0.0f;
				HorizontalDirection.Normalize();
				
				Movement->MaxWalkSpeed = 300.0f;
				AddMovementInput(HorizontalDirection, 1.0f);
			}
			else
			{
				PRINTLOG(TEXT("[Dasan][DirectMove] ❌ CharacterMovement 없음!"));
			}
		}
		
		// 플레이어가 너무 멀어졌는지 체크
		// 직접 이동 모드일 때는 거리 제한을 2배로 늘림
		float MaxDistance = bUseDirectMovement ? (playerMaxDis * 2.0f) : playerMaxDis;
		
		if (Player && GetPlayerDistance(Player) > MaxDistance)
		{
			PRINTLOG(TEXT("[Dasan][TourMove] 플레이어가 너무 멀어짐 (거리: %.1f / 제한: %.1f) - 대기 상태"), 
				GetPlayerDistance(Player), MaxDistance);
			TourStateSystem->SetTourState(ETourState::TourWait);
			
			if (!bUseDirectMovement)  // NavMesh 모드일 때만 AI 중지
			{
				DasanAicontrol->StopMovement();
			}
			
			waitChackTimer = 1.f;
		}
		break;
	}
	
	case ETourState::TourWait:
	{
		waitChackTimer -= 0.1f;
		
		// 1초마다 확인
		if (waitChackTimer <= 0.0f)
		{
			waitChackTimer = 1.f;
			
			if (Player && GetPlayerDistance(Player) <= playerMaxDis)
			{
				PRINTLOG(TEXT("[Dasan][TourWait] 플레이어 복귀 - 다시 이동"));
				TourStateSystem->SetTourState(ETourState::TourMove);
				
				if (CurTargetBuilding)
				{
					if (bUseDirectMovement)
					{
						// 직접 이동 모드는 자동으로 TourMove에서 처리됨
						PRINTLOG(TEXT("[Dasan] 직접 이동 모드 재개"));
					}
					else
					{
						// NavMesh 모드
						FAIMoveRequest MoveRequest;
						MoveRequest.SetGoalActor(CurTargetBuilding);
						MoveRequest.SetAcceptanceRadius(wayPointDis);
						MoveRequest.SetUsePathfinding(true);
						
						DasanAicontrol->MoveTo(MoveRequest);
					}
				}
			}
		}
		break;
	}
	
	case ETourState::TourEnd:
	{
		// 종료 상태 - 다음 퀘스트로 이동
		break;
	}
	
	default:
		break;
	}
}

void ADasanActor::NextQuest()
{
	if (!HasAuthority())
		return;

	if (!DasanAicontrol || !QuestManager)
		return;

	PRINTLOG(TEXT("[Dasan] ========== NextQuest 호출 =========="));

	// 다음 퀘스트가 있는지 확인
	if (!QuestManager->IsHasQuest() || QuestManager->GetCurTarget() == EBuildingType::None)
	{
		PRINTLOG(TEXT("[Dasan] 모든 퀘스트 완료! 투어 종료"));
		
		if (TourStateSystem)
		{
			TourStateSystem->SetTourState(ETourState::TourEnd);
		}
		
		// AI 이동 중지
		DasanAicontrol->StopMovement();
		return;
	}

	// 새로운 목표 건물 찾기
	CurTargetBuilding = FindCurTargetBuilding();
	
	if (!CurTargetBuilding)
	{
		PRINTLOG(TEXT("[Dasan] 다음 목표 건물을 찾지 못함"));
		return;
	}

	float Distance = GetTargetBuildingDistnace();
	PRINTLOG(TEXT("[Dasan] 다음 목표: %s (거리: %.1f)"), *CurTargetBuilding->GetName(), Distance);

	APawn* Player = GetPlayerPawn();
	
	// 플레이어 거리 확인
	if (Player && GetPlayerDistance(Player) > playerMaxDis)
	{
		PRINTLOG(TEXT("[Dasan] 플레이어가 멀리 있음 - 대기 상태"));
		TourStateSystem->SetTourState(ETourState::TourWait);
		waitChackTimer = 1.f;
	}
	else
	{
		PRINTLOG(TEXT("[Dasan] 다음 목표로 이동 시작"));
		TourStateSystem->SetTourState(ETourState::TourMove);

		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(CurTargetBuilding);
		MoveRequest.SetAcceptanceRadius(wayPointDis);
		MoveRequest.SetUsePathfinding(true);

		FPathFollowingRequestResult Result = DasanAicontrol->MoveTo(MoveRequest);
		
		if (Result.Code == EPathFollowingRequestResult::RequestSuccessful)
		{
			PRINTLOG(TEXT("[Dasan] 다음 목적지로 이동 시작: %s"), *CurTargetBuilding->GetName());
			bUseDirectMovement = false;  // NavMesh 모드
		}
		else
		{
			PRINTLOG(TEXT("[Dasan] ========== MoveTo 실패! 직접 이동 모드 활성화 =========="));
			PRINTLOG(TEXT("[Dasan] 실패 코드: %d"), (int32)Result.Code);
			PRINTLOG(TEXT("[Dasan] 목표: %s"), *CurTargetBuilding->GetName());
			PRINTLOG(TEXT("[Dasan] 거리: %.1f"), Distance);
			bUseDirectMovement = true;  // 직접 이동 모드 활성화
			PRINTLOG(TEXT("[Dasan] bUseDirectMovement = TRUE 설정 완료"));
		}
	}
}

float ADasanActor::GetPlayerDistance(class APawn* PlayerPawn) const
{
	if (!PlayerPawn)
		return -1.0f;
		
	return FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
}

APawn* ADasanActor::GetPlayerPawn() const
{
	if (!GetWorld())
		return nullptr;

	return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void ADasanActor::UpdateWidgetState()
{
	if (!DasanWidget)
		return;

	// 현재 상태 가져오기
	ETourState CurTourState = TourStateSystem ? TourStateSystem->GetCurState() : ETourState::None;
	EAnswerState CurAnswerState = AnswerStateSystem ? AnswerStateSystem->GetCurState() : EAnswerState::AnswerListen;

	// 위젯 상태 업데이트
	DasanWidget->UpdateDasanState(DasanState, CurTourState, CurAnswerState);
}

void ADasanActor::OnExecVoiceCommand(EVoiceCommandType InType, AActor* Requester)
{
	if (!HasAuthority())
		return;

	PRINTLOG(TEXT("[Dasan] 음성 명령 수신: %s from %s"),
		*ENUM_TO_NAME(EVoiceCommandType, InType),
		Requester ? *Requester->GetName() : TEXT("Unknown"));

	switch (InType)
	{
	case EVoiceCommandType::Cmd_Summon:
		if (Requester)
		{
			PRINTLOG(TEXT("[Dasan] Cmd_Summon: %s님이 다산을 소환합니다"), *Requester->GetName());
			
			// Requester의 위치로 텔레포트
			FVector PlayerLocation = Requester->GetActorLocation();
			FRotator PlayerRotation = Requester->GetActorRotation();
			
			// 플레이어의 앞쪽으로 200 유닛 떨어진 위치 계산
			FVector TargetLocation = PlayerLocation + (PlayerRotation.Vector() * 200.f);

			// 텔레포트
			TeleportTo(TargetLocation, GetActorRotation(), false, true);

			APlayerActor* RequestPlayer = Cast<APlayerActor>(Requester);
			if (RequestPlayer && RequestPlayer->ChatPlayerSystem)
			{
				FChatMessage ChatMessage(EChatMessageType::NPC, GameString::NPC, TEXT("부르셨습니까?"));
				RequestPlayer->ChatPlayerSystem->ServerRPC_SendChatMessage(ChatMessage);
			}

			UGameSoundManager::Get(GetWorld())->PlaySound2D(EGameSoundType::Cmd_Summon);
		}
		break;

	default:
		break;
	}
}