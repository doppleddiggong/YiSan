// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#include "ADasanActor.h"
#include "UTourStateSystem.h"
#include "UAnswerStateSystem.h"

#include "AIController.h"

#include "AQuestManagerActor.h"
#include "ABuilding.h"
#include "APlayerActor.h"
#include "EBuildingType.h"
#include "UBroadcastManager.h"
#include "EVoiceCommandType.h"
#include "UGameDataManager.h"
#include "FBuildingData.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "FComponentHelper.h"
#include "GameLogging.h"
#include "NavigationPath.h"

#include "Components/WidgetComponent.h"
#include "UDasanWidget.h"
#include "UChatPlayerSystem.h"
#include "UGameSoundManager.h"
#include "YiSan/YiSan.h"
#include "NavigationSystem.h"
#include "UChatBoxWidget.h"
#include "GameFramework/CharacterMovementComponent.h"

#define DASANWIDGET_PATH TEXT("/Game/CustomContents/UI/WBP_DasanWidget.WBP_DasanWidget_C")

ADasanActor::ADasanActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 네트워크 복제 활성화
	bReplicates = true;
	SetReplicateMovement(true);

	bIsTickEnabled = false;

	if (GetMesh())
	{
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	}

	// 건물 퀘스트 인식을 위한 태그 추가
	Tags.Add(GameTags::Dasan);

	// 상태 시스템 컴포넌트 생성
	TourStateSystem = CreateDefaultSubobject<UTourStateSystem>(TEXT("TourStateSystem"));
	AnswerStateSystem = CreateDefaultSubobject<UAnswerStateSystem>(TEXT("AnswerStateSystem"));

	// 위젯 컴포넌트 생성 (머리 위에 표시)
	DasanWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("DasanWidgetComponent"));
	DasanWidgetComp->SetupAttachment(RootComponent);
	DasanWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 125.f));
	DasanWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	DasanWidgetComp->SetDrawSize(FVector2D(300.f, 100.f));

	// 위젯 클래스 설정
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClassFinder(DASANWIDGET_PATH);
	if (WidgetClassFinder.Succeeded())
		DasanWidgetComp->SetWidgetClass(WidgetClassFinder.Class);

	playerMaxDis = 1000.f;
	wayPointDis = 250.f;
	waitChackTimer = 1.f;
}

void ADasanActor::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(bIsTickEnabled);

	bIsPlayingExplainAnim = false;
	// 위젯 캐싱 및 초기화
	if (DasanWidgetComp && DasanWidgetComp->GetWidget())
	{
		DasanWidget = Cast<UDasanWidget>(DasanWidgetComp->GetWidget());
		if (DasanWidget)
		{
			DasanWidget->InitWidget(this);
			PRINTLOG(TEXT("[Dasan] DasanWidget 캐싱 및 초기화 완료"));
		}
		else
		{
			PRINTLOG(TEXT("[Dasan] DasanWidget Cast 실패!"));
		}

		// 클라이언트 접속 시 리플리케이트된 상태로 위젯을 초기화
		// (OnRep는 값이 변경될 때만 호출되므로, 최초 접속 시에는 수동으로 업데이트 필요)
		if (!HasAuthority())
		{
			// 약간의 지연 후 업데이트 (리플리케이션이 완료될 때까지 대기)
			FTimerHandle InitWidgetTimer;
			GetWorldTimerManager().SetTimer(InitWidgetTimer, [this]()
			{
				if (DasanWidget)
				{
					UpdateWidgetState();
					PRINTLOG(TEXT("[DasanWidget] 클라이언트 초기 위젯 상태 업데이트 완료"));
				}
				else
				{
					PRINTLOG(TEXT("[DasanWidget] 클라이언트 초기 업데이트 실패 - Widget is nullptr"));
				}
			}, 0.1f, false);
		}
	}
	else
	{
		PRINTLOG(TEXT("[Dasan] DasanWidgetComp 또는 Widget이 nullptr!"));
	}

	// 값 설정
	playerMaxDis = 1000.f;
	wayPointDis = 250.f;
	waitChackTimer = 1.f;
	
	PRINTLOG(TEXT("========== ADasanActor BeginPlay =========="));
	PRINTLOG(TEXT(" playerMaxDis: %.1f"), playerMaxDis);
	PRINTLOG(TEXT(" wayPointDis: %.1f"), wayPointDis);
	PRINTLOG(TEXT(" HasAuthority: %s"), HasAuthority() ? TEXT("TRUE (Server)") : TEXT("FALSE (Client)"));

	// 시스템 초기화 - 모든 머신에서 실행
	TourStateSystem->InitSystem(this);
	AnswerStateSystem->InitSystem(this);
	PRINTLOG(TEXT(" State Systems 초기화 완료"));

	// 서버 전용 로직
	if (HasAuthority())
	{
		// AI 컨트롤러 가져오기
		DasanAicontrol = Cast<AAIController>(GetController());
		if (!DasanAicontrol)
		{
			PRINTLOG(TEXT(" 블루프린트에서 'Auto Possess AI'를 확인하세요."));
			PRINTLOG(TEXT(" 현재 Controller: %s"), GetController() ? *GetController()->GetName() : TEXT("nullptr"));
			// AI Controller 없어도 일단 진행 (직접 이동 방식 사용)
		}
		else
		{
			PRINTLOG(TEXT(" AI Controller 초기화 성공: %s"), *DasanAicontrol->GetName());

			// AI MoveTo 완료 콜백 바인딩 추가
			DasanAicontrol->ReceiveMoveCompleted.AddDynamic(this, &ADasanActor::OnMoveCompleted);
		}

		QuestManager = AQuestManagerActor::Get(GetWorld());

		// 초기 상태 설정
		DasanState = EDasanState::Tour;

		// 투어 시작
		StartTour();
	}

	// BroadcastManager 이벤트 구독
	BroadcastManager = UBroadcastManager::Get(GetWorld());
	if (BroadcastManager)
	{
		BroadcastManager->OnExecVoiceCommand.AddDynamic(this, &ADasanActor::OnExecVoiceCommand);
		BroadcastManager->OnMessage.AddDynamic(this, &ADasanActor::OnGameMessage);
		PRINTLOG(TEXT(" BroadcastManager 이벤트 구독 성공"));
	}
}

void ADasanActor::OnGameMessage(FString Message)
{
	if (Message == GameMessage::GameStart)
	{
		if (HasAuthority())
		{
			bIsTickEnabled = true;
			OnRep_IsTickEnabled(); // Call OnRep manually on server
		}
		PRINTLOG(TEXT("[Dasan] GameStart 메시지 수신, Tick 활성화"));
	}
}

void ADasanActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 서버에서만 상태 시스템 틱 실행
	if (HasAuthority())
	{
		if ( DasanState == EDasanState::Tour)
		{
			TourStateSystem->UpdateTick(DeltaTime);
		}
	}
}

void ADasanActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADasanActor, DasanState);
	DOREPLIFETIME(ADasanActor, bIsPlayingExplainAnim);
	DOREPLIFETIME(ADasanActor, bIsTickEnabled);
}

void ADasanActor::OnRep_IsTickEnabled()
{
	SetActorTickEnabled(bIsTickEnabled);
}

// AI MoveTo 완료 콜백 - 새로 추가된 함수
void ADasanActor::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
    if (!HasAuthority())
        return;

    // TourStateSystem 등 사용 전에 체크
    switch (Result)
    {
    case EPathFollowingResult::Success:
        PRINTLOG(TEXT(" AI MoveTo 성공 - 목적지 도착"));

        // Tour 상태일 때만 처리
        if (DasanState == EDasanState::Tour)
        {
            if (TourStateSystem)
            {
                // 건물 도착 시 TourExplain 상태로 전환 (관광 시작)
                TourStateSystem->SetTourState(ETourState::TourExplain);
                PRINTLOG(TEXT("[Tour] 건물 도착 - TourExplain 상태 시작"));
            }
            else
            {
                PRINTLOG(TEXT("[WARN] OnMoveCompleted: TourStateSystem이 nullptr입니다."));
            }
        }
        break;

    case EPathFollowingResult::Blocked:
        PRINTLOG(TEXT(" AI MoveTo 차단됨 - 재시도"));
        // 0.5초 후 재시도
        if (CurTargetBuilding && DasanAicontrol)
        {
            FTimerHandle RetryTimer;
            GetWorldTimerManager().SetTimer(RetryTimer, [this]()
            {
                if (CurTargetBuilding && DasanAicontrol)
                {
                    FVector TargetPos = CurTargetBuilding->GetActorLocation();
                    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

                    if (NavSys)
                    {
                        FNavLocation TargetNavLoc;
                        if (NavSys->ProjectPointToNavigation(TargetPos, TargetNavLoc, FVector(5000, 5000, 5000)))
                        {
                        	auto Result = this->AIMoveToLoc(TargetNavLoc.Location, wayPointDis, true);
                        }
                    }
                }
                else
                {
                    PRINTLOG(TEXT("Retry MoveTo 실패: CurTargetBuilding 또는 DasanAicontrol이 nullptr입니다."));
                }
            }, 0.5f, false);
        }
        else
        {
            PRINTLOG(TEXT("Blocked 처리: CurTargetBuilding 또는 DasanAicontrol이 nullptr입니다."));
        }
        break;

    case EPathFollowingResult::Aborted:
        // PRINTLOG(TEXT(" AI MoveTo 중단됨"));
        break;

    case EPathFollowingResult::Invalid:
        PRINTLOG(TEXT(" AI MoveTo 실패 - 경로 없음"));
        break;

    default:
        break;
    }
}

// RepNotify 함수
void ADasanActor::OnRep_DasanState()
{
	PRINTLOG(TEXT("DasanActor MainState changed: %s"), *ENUM_TO_NAME(EDasanState, DasanState));

	// 클라이언트에서 메인 상태가 복제되었으므로 위젯 업데이트
	UpdateWidgetState();
}

float ADasanActor::GetTargetBuildingDistnace()
{
	if (!CurTargetBuilding)
	{
		PRINTLOG(TEXT(" CurTargetBuilding이 nullptr임"));
		return -1.0f;
	}
	return FVector::Dist(this->GetActorLocation(), this->CurTargetBuilding->GetActorLocation());
}

void ADasanActor::StartTour()
{
    PRINTLOG(TEXT("========== StartTour 호출 =========="));

    if (!HasAuthority())
    {
        PRINTLOG(TEXT("클라이언트에서 호출됨"));
        return;
    }

    // 첫 번째 목표 건물 찾기
    CurTargetBuilding = FindCurTargetBuilding();

    if (CurTargetBuilding)
    {
        float Distance = GetTargetBuildingDistnace();
        PRINTLOG(TEXT(" 타겟 건물 발견: %s (거리: %.1f)"), *CurTargetBuilding->GetName(), Distance);

        // 상태를 먼저 설정
        DasanState = EDasanState::Tour;

        // 투어 시작 시 플레이어 거리 체크
        APawn* Player = GetPlayerPawn();

    	const bool bHasPlayer = (Player != nullptr);
    	const float PlayerDistance = bHasPlayer ? GetPlayerDistance(Player) : -1.0f;

    	if (!bHasPlayer)
    	{
    		PRINTLOG(TEXT(" 유효한 플레이어가 없어 대기 상태(TourWait)에서 시작함"));
    	}

    	if (!bHasPlayer || PlayerDistance > playerMaxDis)
    	{
    		if (PlayerDistance > playerMaxDis && bHasPlayer)
    		{
    			PRINTLOG(TEXT(" 플레이어가 너무 멀리 있음(거리: %.1f). 대기 상태(TourWait)에서 시작함"), PlayerDistance);
    		}

    		TourStateSystem->SetTourState(ETourState::TourWait);
            waitChackTimer = 1.f;
        }
        else
        {
        	PRINTLOG(TEXT(" 플레이어가 근처에 있음(거리: %.1f). 이동 상태(TourMove) 시작함"), PlayerDistance);

        	TourStateSystem->SetTourState(ETourState::TourMove);

            // AI Controller로 이동 시작 (NavMesh 위치로)
            if (DasanAicontrol && CurTargetBuilding)
            {
                FVector TargetPos = CurTargetBuilding->GetActorLocation();
                UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
                FNavLocation TargetNavLoc;
                if (NavSys->ProjectPointToNavigation(TargetPos, TargetNavLoc, FVector(5000, 5000, 5000)))
                {
                	auto Result = this->AIMoveToLoc(TargetNavLoc.Location, wayPointDis, true);
                	if (Result.Code == EPathFollowingRequestResult::RequestSuccessful)
                	    PRINTLOG(TEXT(" AI MoveTo 시작 성공 (NavMesh 위치)"));
                	else
                	    PRINTLOG(TEXT(" AI MoveTo 실패: %d"), (int32)Result.Code);
				}
            }
        }

        PRINTLOG(TEXT(" StartTour 완료 - DasanState: %s, TourState: %s"), *ENUM_TO_NAME(EDasanState, DasanState),
            TourStateSystem ? *ENUM_TO_NAME(ETourState, TourStateSystem->GetCurState()) : TEXT("Unknown"));
    }
    else
    {
        PRINTLOG(TEXT(" [CRITICAL] 타겟 건물을 찾을 수 없음"));
        DasanState = EDasanState::Tour;
        if (TourStateSystem)
            TourStateSystem->SetTourState(ETourState::TourEnd);
    }
}

ABuilding* ADasanActor::FindCurTargetBuilding() const
{
	if (!QuestManager)
	{
		return CurTargetBuilding; // 기존 타깃 유지
	}

	EBuildingType TargetType = QuestManager->GetCurrentTarget();
	if (TargetType == EBuildingType::None)
	{
		return CurTargetBuilding;
	}

	// 월드에서 모든 ABuilding 검색
	auto FoundBuildings = FComponentHelper::GetAllOfClass<ABuilding>(GetWorld());
	PRINTLOG(TEXT(" 월드에서 건물 검색 중... (총 %d개 발견)"), FoundBuildings.Num());

	for (auto Building : FoundBuildings)
	{
		if (Building && Building->BuildingType == TargetType)
		{
			PRINTLOG(TEXT(" 타겟 건물 발견: %s (타입 일치)"), *Building->GetName());
			return Building;
		}
	}

	PRINTLOG(TEXT("[WARN] FindCurTargetBuilding: 타겟 건물을 찾을 수 없음"));

	// 실패 시에도 기존 타깃 유지
	return CurTargetBuilding;
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

	// Answer 상태일 때 다른 상태로의 전환을 막는 로직 추가
	if (AnswerStateSystem &&
		AnswerStateSystem->IsAnswerSessionActive() &&
		InMainState != EDasanState::Answer )
	{
		PRINTLOG(TEXT("TransitionToState: Ask상태중에는 다른 상태로 전환할 수 없습니다."));
		return;
	}

    PRINTLOG(TEXT(" 상태 전환: [%s] → [%s]"), *ENUM_TO_NAME(EDasanState, DasanState), *ENUM_TO_NAME(EDasanState, InMainState));

    // 메인 상태 변경
    DasanState = InMainState;

    // 위젯 업데이트 (메인 상태가 바뀌었으므로)
    UpdateWidgetState();

    switch (InMainState)
    {
	    case EDasanState::Tour:
	    {
    		if (!CurTargetBuilding)
    		{
    			PRINTLOG(TEXT("Tour 전환 시 CurTargetBuilding이 비어있어 FindCurTargetBuilding() 호출"));
    			CurTargetBuilding = FindCurTargetBuilding();
    		}
    			
	        const FString TargetName = CurTargetBuilding ? CurTargetBuilding->GetName() : TEXT("None");
	        PRINTLOG(TEXT(" Tour 상태 시작 - 목표: %s"), *TargetName);

	        if (TourStateSystem)
	        {
	            TourStateSystem->SetTourState(ETourState::TourMove);
	        }

	        // AI Controller로 이동 시작 (NavMesh 위치로)
	        if (DasanAicontrol && CurTargetBuilding)
	        {
	            FVector TargetPos = CurTargetBuilding->GetActorLocation();
	            UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

	            if (NavSys)
	            {
	                FNavLocation TargetNavLoc;
	                if (NavSys->ProjectPointToNavigation(TargetPos, TargetNavLoc, FVector(5000, 5000, 5000)))
	                {
                		auto Result = this->AIMoveToLoc(TargetNavLoc.Location, wayPointDis, true);
	                    PRINTLOG(TEXT(" AI MoveTo 시작 (NavMesh 위치)"));
	                }
	            }
	        }
	        else
	        {
	            PRINTLOG(TEXT(" AI MoveTo를 시작하지 않음 (직접 이동 또는 대기 모드)"));
	        }
	    }
	    break;
    	
	    case EDasanState::Answer:
	    {
	        PRINTLOG(TEXT("[EDasanState::Answer] Answer 상태 시작"));
	        if (AnswerStateSystem)
	        {
	            AnswerStateSystem->SetCurState(EAnswerState::AnswerListen);
	        }
	    }
    	break;

    	default:
    		break;
    }
}

void ADasanActor::NextQuest()
{
    if (!HasAuthority())
        return;

    if (!DasanAicontrol)
        return;

    if (!QuestManager)
        return;

    // 퀘스트 매니저가 준비되지 않거나 타겟이 None이면 이동 중단
    if (!QuestManager->HasActiveQuest() || QuestManager->GetCurrentTarget() == EBuildingType::None)
    {
        PRINTLOG(TEXT(" NextQuest: 유효한 퀘스트가 없거나 TargetType이 None입니다."));
        TourStateSystem->SetTourState(ETourState::TourEnd);
        return;
    }

    // 새로운 목표 건물 찾기
    CurTargetBuilding = FindCurTargetBuilding();
    PRINTLOG(TEXT("NextQuest: TargetBuilding = %s"), 
        CurTargetBuilding ? *CurTargetBuilding->GetName() : TEXT("None"));

    if (CurTargetBuilding)
    {
        float Distance = GetTargetBuildingDistnace();
        PRINTLOG(TEXT(" 다음 목표 건물까지 거리: %.1f"), Distance);

        APawn* Player = GetPlayerPawn();
        if (Player && GetPlayerDistance(Player) > playerMaxDis)
        {
            PRINTLOG(TEXT(" 플레이어가 너무 멀리 있음. 대기 상태 시작"));
            TourStateSystem->SetTourState(ETourState::TourWait);
            waitChackTimer = 1.f;
        }
        else
        {
            PRINTLOG(TEXT(" 플레이어가 근처에 있음. 이동 상태 시작"));
            TourStateSystem->SetTourState(ETourState::TourMove);

            FVector TargetPos = CurTargetBuilding->GetActorLocation();
            UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

            if (NavSys)
            {
                FNavLocation TargetNavLoc;
                if (NavSys->ProjectPointToNavigation(TargetPos, TargetNavLoc, FVector(5000, 5000, 5000)))
                {
					auto Result = this->AIMoveToLoc(TargetNavLoc.Location, wayPointDis, true);
                	if (Result.Code == EPathFollowingRequestResult::RequestSuccessful)
                		PRINTLOG(TEXT(" 다음 목적지로 이동 시작: %s (NavMesh 위치)"), *CurTargetBuilding->GetName());
                	else
                		PRINTLOG(TEXT("[WARN] MoveTo 실패: 코드 %d"), (int32)Result.Code);
                }
            }
        }
    }
    else
    {
        PRINTLOG(TEXT("[WARN] NextQuest: CurTargetBuilding이 nullptr입니다. 이동 불가"));
        TourStateSystem->SetTourState(ETourState::TourEnd);
    }
}


FPathFollowingRequestResult ADasanActor::AIMoveToLoc(
	const FVector& InGoalLocation,
	const float InAcceptanceRadius,
	const bool bPathfinding )
{
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(InGoalLocation);
	MoveRequest.SetAcceptanceRadius(InAcceptanceRadius);
	MoveRequest.SetUsePathfinding(bPathfinding);

	DebugDrawPath(InGoalLocation);
	
	return DasanAicontrol->MoveTo(MoveRequest);
}

FPathFollowingRequestResult ADasanActor::AIMoveToActor(
	const AActor* Actor,
	const float InAcceptanceRadius,
	const bool bPathfinding )
{
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Actor);
	MoveRequest.SetAcceptanceRadius(InAcceptanceRadius);
	MoveRequest.SetUsePathfinding(bPathfinding);
	MoveRequest.SetProjectGoalLocation(true);

	DebugDrawPath(Actor->GetActorLocation());
	
	return DasanAicontrol->MoveTo(MoveRequest);
}

bool ADasanActor::IsNearTargetBuilding()
{
	auto building = GetCurTargetBuilding();
	if ( building == nullptr )
		return false;

	float Distance = FVector::Dist( this->GetActorLocation(), GetCurTargetBuilding()->GetActorLocation() );
	return Distance <= 100.0f;
}

float ADasanActor::GetPlayerDistance(APawn* PlayerPawn) const
{
	if (!PlayerPawn)
	{
		return -1.0f;
	}
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
	// 캐싱된 위젯 사용
	if (!DasanWidget)
	{
		PRINTLOG(TEXT("[Dasan] UpdateWidgetState - DasanWidget is nullptr!"));
		return;
	}

	PRINTLOG(TEXT("[Dasan] UpdateWidgetState - DasanState: %s, TourState: %s, AnswerState: %s (Authority: %s)"),
		*ENUM_TO_NAME(EDasanState, DasanState),
		*ENUM_TO_NAME(ETourState, TourStateSystem->GetCurState()),
		*ENUM_TO_NAME(EAnswerState, AnswerStateSystem->GetCurState()),
		HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));

	// 컨텍스트 데이터 수집
	FString BuildingName;
	FString PlayerName;
	int32 CurPlayers = 0;
	int32 MaxPlayers = 0;

	// 건물 이름 가져오기 (Tour 상태일 때)
	if (DasanState == EDasanState::Tour && CurTargetBuilding)
	{
		if (auto DM = UGameDataManager::Get(GetWorld()))
		{
			FBuildingData BuildingData;
			if (DM->GetBuildingData(CurTargetBuilding->BuildingType, BuildingData))
				BuildingName = BuildingData.name;
		}
	}


	// 플레이어 카운트 (TourWait 상태일 때)
	if (DasanState == EDasanState::Tour && TourStateSystem->GetCurState() == ETourState::TourWait)
	{
		auto Players = FComponentHelper::GetAllOfClass<APlayerActor>(GetWorld());
		MaxPlayers = Players.Num();
		FVector DasanLocation = GetActorLocation();

		for (auto Player : Players)
		{
			float Distance = FVector::Dist(DasanLocation, Player->GetActorLocation());
			if (Distance <= 500.0f) // PlayerDetectionRadius
			{
				CurPlayers++;
			}
		}
	}

	// 플레이어 이름 가져오기 (Answer 상태일 때)
	if (DasanState == EDasanState::Answer)
	{
		if (auto Player = Cast<APlayerActor>(GetPlayerPawn()))
		{
			PlayerName = Player->GetPlayerDisplayName();
		}
	}

	// 위젯 상태 업데이트
	DasanWidget->UpdateDasanState(DasanState, TourStateSystem->GetCurState(), AnswerStateSystem->GetCurState(),
		BuildingName, PlayerName, CurPlayers, MaxPlayers);
}

void ADasanActor::OnExecVoiceCommand(EVoiceCommandType InType, AActor* Requester)
{
	if (!HasAuthority())
		return;

	PRINTLOG(TEXT("[Dasan] 음성 명령 수신: %s from %s"), *ENUM_TO_NAME(EVoiceCommandType, InType),
		Requester ? *Requester->GetName() : TEXT("Unknown"));

	if ( InType == EVoiceCommandType::Cmd_Approach )
	{
		if (Requester == nullptr)
			return;

		APlayerActor* RequestPlayer = Cast<APlayerActor>(Requester);
		if (!RequestPlayer)
			return;

		PRINTLOG(TEXT("[Dasan] Cmd_Approach: %s님이 다산에게 이동합니다"), *RequestPlayer->GetName());

		// Dasan의 위치와 회전
		FVector DasanLocation = GetActorLocation();
		FRotator DasanRotation = GetActorRotation();

		// Dasan의 앞쪽으로 200 유닛 떨어진 위치 계산 (Player가 이동할 위치)
		FVector TargetLocation = DasanLocation + (DasanRotation.Vector() * 200.f);

		// Player를 Dasan 앞으로 텔레포트
		RequestPlayer->TeleportTo(TargetLocation, RequestPlayer->GetActorRotation(), false, true);

		// 채팅 메시지
		if (RequestPlayer->ChatPlayerSystem)
		{
			FChatMessage ChatMessage(EChatMessageType::NPC, -1, GameString::NPC, TEXT("어서 오십시오!"));
			RequestPlayer->ChatPlayerSystem->ServerRPC_SendChatMessage(ChatMessage);
		}

		UGameSoundManager::Get(GetWorld())->PlaySound2D(EGameSoundType::Cmd_Summon);
	}
}

void ADasanActor::DebugDrawPath(const FVector& GoalLocation)
{
	if (!bEnableDebugDraw)
		return;
	
	if (!HasAuthority())
		return; // 서버에서만 실행

	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(
		GetWorld(),
		GetActorLocation(),
		GoalLocation
	);

	if (NavPath && NavPath->PathPoints.Num() > 1)
		MulticastRPC_DrawDebugPath(NavPath->PathPoints);
}



void ADasanActor::ShowExplainDialog(const FString& ExplainText)
{
	if (!HasAuthority())
	{
		PRINTLOG(TEXT("[Dasan] ShowExplainDialog - No authority, ignoring"));
		return;
	}

	PRINTLOG(TEXT("[Dasan] ShowExplainDialog - Text: %s"), *ExplainText);

	// 모든 클라이언트에게 RPC 전송
	MulticastRPC_ShowExplainDialog(ExplainText);
}


void ADasanActor::HideExplainDialog()
{
	if (!HasAuthority())
	{
		PRINTLOG(TEXT("[Dasan] HideExplainDialog - No authority, ignoring"));
		return;
	}

	PRINTLOG(TEXT("[Dasan] HideExplainDialog"));

	// 모든 클라이언트에게 RPC 전송
	MulticastRPC_HideExplainDialog();
}


void ADasanActor::MulticastRPC_DrawDebugPath_Implementation(const TArray<FVector>& PathPoints)
{
	FlushPersistentDebugLines(GetWorld());

	if (PathPoints.Num() > 1)
	{
		for (int32 i = 0; i < PathPoints.Num() - 1; i++)
		{
			FVector Start = PathPoints[i];
			FVector End   = PathPoints[i + 1];

			// 선으로 경로 표시
			DrawDebugLine(GetWorld(), Start, End, FColor::Green, true, 0.0f, 0.0f, 10.f);
		}
	}
}

void ADasanActor::MulticastRPC_ShowExplainDialog_Implementation(const FString& ExplainText)
{
	// 모든 클라이언트에서 실행됨
	if (!DasanWidget)
	{
		PRINTLOG(TEXT("[Dasan] MulticastRPC_ShowExplainDialog - DasanWidget is nullptr!"));
		return;
	}

	PRINTLOG(TEXT("[Dasan] MulticastRPC_ShowExplainDialog - Showing dialog: %s"), *ExplainText);

	// 위젯에 다이얼로그 표시 요청
	DasanWidget->ShowExplainDialog(ExplainText);
}


void ADasanActor::MulticastRPC_HideExplainDialog_Implementation()
{
	// 모든 클라이언트에서 실행됨
	if (!DasanWidget)
	{
		PRINTLOG(TEXT("[Dasan] MulticastRPC_HideExplainDialog - DasanWidget is nullptr!"));
		return;
	}

	PRINTLOG(TEXT("[Dasan] MulticastRPC_HideExplainDialog") );

	// 위젯에 다이얼로그 표시 요청
	DasanWidget->HideExplainDialog();
}

void ADasanActor::SendDasanChatMessage(const FString& Message)
{
	if (HasAuthority())
	{
		MulticastRPC_SendDasanChatMessage(Message);
	}
}

void ADasanActor::MulticastRPC_SendDasanChatMessage_Implementation(const FString& Message)
{
	// 현재 클라이언트의 로컬 플레이어 컨트롤러를 가져옴
	auto* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
		return;

	// 로컬 컨트롤러가 조종 중인 Pawn을 APlayerActor 타입으로 변환
	auto* Player = Cast<APlayerActor>(PC->GetPawn());
	if (!Player)
		return;

	// 플레이어가 보유한 채팅 UI 위젯 참조
	auto* ChatBox = Player->ChatBoxWidget.Get();
	if (!ChatBox)
		return;

	// 다산의 채팅 메시지 생성
	FChatMessage ChatMessage(EChatMessageType::NPC, -1, GameString::NPC, Message);

	// 로컬 클라이언트의 채팅창에 메시지 출력
	ChatBox->AddChatMessage(ChatMessage);
}


// TODO 애님관련 입니다


float ADasanActor::GetGroundSpeed()
{
	if(UCharacterMovementComponent* move = GetCharacterMovement())
	{
		FVector velo = move->Velocity;
		velo.Z = 0.0f;
		return velo.Size();
	}
	return 0.0f;
}

bool ADasanActor::IsExplaining() 
{
	if (TourStateSystem && TourStateSystem->GetCurState() == ETourState::TourExplain)
	{
		return true;
	}
	return bIsPlayingExplainAnim;
}


void ADasanActor::StartExplainAnim()
{
	if (HasAuthority())
	{
		bIsPlayingExplainAnim = true;
		PRINTLOG(TEXT("설명 애니메이션 시작"));
	}
}

void ADasanActor::EndExplainAnim()
{
	if (HasAuthority())
	{
		bIsPlayingExplainAnim = false;
		PRINTLOG(TEXT("설명 애니메이션 끝"));
	}
}