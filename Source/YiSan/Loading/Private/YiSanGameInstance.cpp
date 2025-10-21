// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "YiSan/Loading/Public/YiSanGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "ContentStreaming.h"
#include "GameLogging.h"
#include "Streaming/StreamingWorldSubsystemInterface.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#if WITH_EDITOR
#include "ShaderCompiler.h"
#endif
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

UYiSanGameInstance::UYiSanGameInstance()
{
    // 로딩 위젯 클래스 설정 (블루프린트에서도 설정 가능)
    // LoadingWidgetClass = ...;
}

void UYiSanGameInstance::Init()
{
    Super::Init();
    UE_LOG(LogTemp, Log, TEXT("[YiSan GameInstance] Initialized"));

    IOnlineSubsystem* subsys = Online::GetSubsystem(GetWorld());
    if (subsys)
    {
        //서브시스템의 인터페이스를 가져오자
        sessionInterface = subsys->GetSessionInterface();
        //세션생성 성공시 호출되는 함수 등록
        sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UYiSanGameInstance::OnCreateSessionComplete);
        //세션조회 성공시 호출되는 함수 등록
        sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UYiSanGameInstance::OnFindSessionComplete);
        //세션참여 성공시 호출되는 함수 등록
        sessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UYiSanGameInstance::OnJoinSessionComplete);
    }
}

void UYiSanGameInstance::LoadLevelWithLoadingScreen(FName InTargetLevelName)
{
    if (InTargetLevelName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("[YiSan] Invalid target level name!"));
        return;
    }

    TargetLevelName = InTargetLevelName;
    UE_LOG(LogTemp, Log, TEXT("[YiSan] Starting level transition to: %s"), *TargetLevelName.ToString());

    // 로딩 UI 표시
    ShowLoadingScreen();

    // LoadingMap으로 먼저 이동
    UGameplayStatics::OpenLevel(this, FName("LoadingMap"));

    // LoadingMap이 로드된 후 타겟으로 전환
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimerForNextTick([this]()
        {
            if (UWorld* CurrentWorld = GetWorld())
            {
                FTimerHandle TransitionTimer;
                CurrentWorld->GetTimerManager().SetTimer(
                    TransitionTimer,
                    [this]()
                    {
                        // 로딩 화면을 보여준 후 타겟 레벨로 전환
                        UGameplayStatics::OpenLevel(this, TargetLevelName);
                        
                        // 타겟 레벨이 로드되면 로딩 화면 숨기기
                        // (타겟 레벨의 BeginPlay 등에서 처리)
                    },
                    1.5f,  // 로딩 화면을 1.5초 보여줌
                    false
                );
            }
        });
    }
}


// ==================== Step 1: 로딩 레벨로 이동 ====================

void UYiSanGameInstance::Step1_MoveToLoadingLevel()
{
    UE_LOG(LogTemp, Log, TEXT("[YiSan Step 1] Moving to loading level"));

    // 로딩 스크린 표시
    ShowLoadingScreen();

    // 로딩 레벨로 이동 (동기)
    // 주의: 로딩 레벨은 가벼워야 합니다!
    UGameplayStatics::OpenLevel(this, FName("/Game/CustomContents/Maps/LoadingMap"));

    // 로딩 레벨이 로드되면 Step2 실행
    FTimerHandle DelayTimer;
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimerForNextTick([this]()
        {
            // 다음 프레임에 Step2 실행을 위한 타이머 설정
            if (UWorld* CurrentWorld = GetWorld())
            {
                FTimerHandle SecondDelayTimer;
                CurrentWorld->GetTimerManager().SetTimer(
                    SecondDelayTimer,
                    this,
                    &UYiSanGameInstance::Step2_StartLoadingTargetLevel,
                    0.5f,  // 로딩 레벨 안정화 대기
                    false
                );
            }
        });
    }
}

// ==================== Step 2: 타겟 레벨 로드 시작 ====================

void UYiSanGameInstance::Step2_StartLoadingTargetLevel()
{
    UE_LOG(LogTemp, Log, TEXT("[YiSan Step 2] Starting to load target level: %s"), *TargetLevelName.ToString());
    
    UGameplayStatics::OpenLevel(this, TargetLevelName);
}

// ==================== Step 3: 레벨 로드 완료 ====================

void UYiSanGameInstance::Step3_OnLevelLoaded()
{
    UE_LOG(LogTemp, Log, TEXT("[YiSan Step 3] Target level loaded into memory"));
    bLevelLoaded = true;

    // 이제 리소스(텍스처, 셰이더 등)가 완전히 준비될 때까지 대기
    Step4_CheckResources();
}

// ==================== Step 4: 리소스 스트리밍 체크 ====================

void UYiSanGameInstance::Step4_CheckResources()
{
    UE_LOG(LogTemp, Log, TEXT("[YiSan Step 4] Checking resources..."));

    if (UWorld* World = GetWorld())
    {
        // 주기적으로 리소스 상태 확인
        World->GetTimerManager().SetTimer(
            ResourceCheckTimer,
            [this]()
            {
                bool bTexturesReady = CheckTextureStreaming();
                bool bShadersReady = CheckShaderCompilation();
                bool bWPReady = CheckWorldPartition();
                PRINTLOG(TEXT("Resource Check: Textures Ready: %s, Shaders Ready: %s, World Partition Ready: %s"),(bTexturesReady ? TEXT("True") : TEXT("False")),
                     (bShadersReady ? TEXT("True") : TEXT("False")),
                     (bWPReady ? TEXT("True") : TEXT("False")));

                // 진행률 로그
                UE_LOG(LogTemp, Verbose, TEXT("[YiSan] Resources - Textures: %d, Shaders: %d, WP: %d"),
                    bTexturesReady, bShadersReady, bWPReady);

                if (bTexturesReady && bShadersReady && bWPReady)
                {
                    // 모두 준비 완료!
                    if (UWorld* CurrentWorld = GetWorld())
                    {
                        CurrentWorld->GetTimerManager().ClearTimer(ResourceCheckTimer);
                    }
                    Step5_TransitionToTarget();
                }
             },
            0.1f,  // 0.1초마다 체크
            true   // 반복
        );
    }
}

bool UYiSanGameInstance::CheckTextureStreaming()
{
    // IStreamingManager::Get()을 통해 스트리밍 매니저에 접근합니다.
    IStreamingManager* StreamingManager = &IStreamingManager::Get();
    
    if (StreamingManager)
    {
        // 텍스처 스트리밍 업데이트
        StreamingManager->UpdateResourceStreaming(0.0f, false);
        
        // 모든 리소스 스트리밍 시도
        float StreamingPercentage = StreamingManager->StreamAllResources(0.0f);
        
        // 1.0이면 완료
        return StreamingPercentage >= 1.0f;
    }
    
    // 스트리밍 매니저가 없으면 완료로 간주
    return true;
}

bool UYiSanGameInstance::CheckShaderCompilation()
{
#if WITH_EDITOR
    // 에디터에서만 셰이더 컴파일 체크
    if (GShaderCompilingManager)
    {
        int32 RemainingJobs = GShaderCompilingManager->GetNumRemainingJobs();
        
        if (RemainingJobs > 0)
        {
            UE_LOG(LogTemp, Verbose, TEXT("[YiSan] Waiting for %d shader compilation jobs"), RemainingJobs);
            return false;
        }
    }
#endif
    
    // 패키징된 빌드에서는 항상 true (셰이더가 미리 컴파일됨)
    return true;
}

bool UYiSanGameInstance::CheckWorldPartition()
{
    if (UWorld* World = GetWorld())
    {
        if (UWorldPartitionSubsystem* WPS = World->GetSubsystem<UWorldPartitionSubsystem>())
        {
            // World Partition 스트리밍 완료 여부 확인
            bool bCompleted = WPS->IsStreamingCompleted(nullptr);
            
            if (!bCompleted)
            {
                UE_LOG(LogTemp, Verbose, TEXT("[YiSan] World Partition streaming in progress..."));
            }
            
            return bCompleted;
        }
    }
    
    // World Partition이 없으면 완료로 간주
    return true;
}

// ==================== Step 5: 타겟 레벨로 전환 ====================

void UYiSanGameInstance::Step5_TransitionToTarget()
{
    UE_LOG(LogTemp, Log, TEXT("[YiSan Step 5] All resources ready! Transitioning to target level"));

    if (UWorld* World = GetWorld())
    {
        // 약간의 딜레이 후 전환 (안정화)
        FTimerHandle FinalDelayTimer;
        World->GetTimerManager().SetTimer(
            FinalDelayTimer,
            [this]()
            {
                // 로딩 스크린 숨김
                HideLoadingScreen();

                // 타겟 레벨로 완전 전환
                // 이제 OpenLevel을 사용하여 로딩 레벨을 언로드하고 타겟 레벨만 남김
                UGameplayStatics::OpenLevel(this, TargetLevelName);

                UE_LOG(LogTemp, Log, TEXT("[YiSan] Level transition complete!"));
            },
            0.3f,  // 300ms 후 전환
            false
        );
    }
}

// ==================== UI 관리 ====================

void UYiSanGameInstance::ShowLoadingScreen()
{
    if (LoadingWidgetClass && !LoadingWidget)
    {
        if (UWorld* World = GetWorld())
        {
            LoadingWidget = CreateWidget<UUserWidget>(World, LoadingWidgetClass);
            if (LoadingWidget)
            {
                LoadingWidget->AddToViewport(9999);  // 최상위 레이어
                UE_LOG(LogTemp, Log, TEXT("[YiSan] Loading screen shown"));
            }
        }
    }
}

void UYiSanGameInstance::HideLoadingScreen()
{
    if (LoadingWidget)
    {
        LoadingWidget->RemoveFromParent();
        LoadingWidget = nullptr;
        UE_LOG(LogTemp, Log, TEXT("[YiSan] Loading screen hidden"));
    }
}


// ==================== Network 관리 ====================


void UYiSanGameInstance::CreateMySession(FString displayName, int32 playerCount)
{
    FOnlineSessionSettings sessionSettings;
    FName subsysName = Online::GetSubsystem(GetWorld())->GetSubsystemName();
    sessionSettings.bIsLANMatch = subsysName.IsEqual(FName(TEXT("NULL")));
    sessionSettings.bUseLobbiesIfAvailable = true;
    sessionSettings.bUsesPresence = true;
    sessionSettings.bShouldAdvertise = true;
    sessionSettings.NumPublicConnections = playerCount;
    sessionSettings.Set(FName("DP_NAME"), displayName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
    sessionInterface->CreateSession(*netID, FName(displayName), sessionSettings);
    UE_LOG(LogTemp, Warning, TEXT("서브시스템 : %s"), *subsysName.ToString());
}

void UYiSanGameInstance::OnCreateSessionComplete(FName sessionName, bool success)
{
    if (success)
    {
        UE_LOG(LogTemp, Warning, TEXT("세션 : %s 성공"), *sessionName.ToString());
        GetWorld()->ServerTravel(TEXT("/Game/CustomContents/Maps/StartLevel"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("세션 : %s 실패"), *sessionName.ToString());
    }
}

void UYiSanGameInstance::FindOtherSession()
{
    UE_LOG(LogTemp, Warning, TEXT("세션 조회 시작"));
    //sessionSearch 만들자
    sessionSearch = MakeShared<FOnlineSessionSearch>();
    FName subsysName = Online::GetSubsystem(GetWorld())->GetSubsystemName();
    sessionSearch->bIsLanQuery = subsysName.IsEqual(FName(TEXT("NULL")));
    sessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
    sessionSearch->MaxSearchResults = 100;
    sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}

void UYiSanGameInstance::OnFindSessionComplete(bool success)
{
    UE_LOG(LogTemp, Warning, TEXT("세션 조회 끝"));
    if (success)
    {
        auto results = sessionSearch->SearchResults;
        for (int32 i = 0; i < results.Num(); i++)
        {
            //세션이름담을변수
            FString displayName;
            results[i].Session.SessionSettings.Get(FName(TEXT("DP_NAME")), displayName);
            UE_LOG(LogTemp, Warning, TEXT("세션 : %i, 이름 : %s"), i, *displayName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("세션 조회 실패"));
    }
}

void UYiSanGameInstance::JoinOtherSession(int32 sessionIndex)
{
    auto results = sessionSearch->SearchResults;
    if (results.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No sessions found! Cannot join."));
        return;
    }
    results[sessionIndex].Session.SessionSettings.bUseLobbiesIfAvailable = true;
    results[sessionIndex].Session.SessionSettings.bUsesPresence = true;

    FString displayName;
    results[sessionIndex].Session.SessionSettings.Get(FName(TEXT("DP_NAME")), displayName);

    sessionInterface->JoinSession(0, FName(displayName), results[sessionIndex]);
}

void UYiSanGameInstance::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
    if (result == EOnJoinSessionCompleteResult::Success)
    {
        FString url;
        sessionInterface->GetResolvedConnectString(sessionName, url);
        UE_LOG(LogTemp, Warning, TEXT("URL : %s"), *url)
        //서버가있는 맵으로 이동 (최초1회)
        APlayerController* pc = GetWorld()->GetFirstPlayerController();
        pc->ClientTravel(url, TRAVEL_Absolute);
    }
}
