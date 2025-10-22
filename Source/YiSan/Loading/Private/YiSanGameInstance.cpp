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
    //이미 로딩 중이면 새로운 요청을 무시합니다. (무한 루프 방지)
    if (bIsLoadingLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("[YiSan] Already in loading process. Ignoring new request to load: %s"), *InTargetLevelName.ToString());
        return;
    }

    if (InTargetLevelName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("[YiSan] Invalid target level name!"));
        return;
    }

    //로딩 시작 플래그 설정
    bIsLoadingLevel = true;
    TargetLevelName = InTargetLevelName;
    PRINTLOG(TEXT("레벨로 이동: %s"), *TargetLevelName.ToString());

    // step1 으로 이동한다
    Step1_MoveToLoadingLevel();
    
}



// ==================== Step 1: 로딩 레벨로 이동 ====================

void UYiSanGameInstance::Step1_MoveToLoadingLevel()
{
    UE_LOG(LogTemp, Warning, TEXT("[YiSan Step 1] Moving to loading level"));

    // 로딩 스크린 표시
    ShowLoadingScreen();
    //step 1 에서 레벨 넘기기만 하기
    UGameplayStatics::OpenLevel(this, FName("/Game/CustomContents/Maps/LoadingMap"));
}

void UYiSanGameInstance::OnLoadingMapReady()
{
    PRINTLOG(TEXT("로딩 안정화"));
    if (UWorld* World = GetWorld())
    {
        // 안정화 용
        FTimerHandle TimerHandle;
        GetTimerManager().SetTimer(TimerHandle,this,&UYiSanGameInstance::Step2_StartLoadingTargetLevel,0.5,false);
    }
}

// ==================== Step 2: 타겟 레벨 로드 시작 ====================

void UYiSanGameInstance::Step2_StartLoadingTargetLevel()
{
    UE_LOG(LogTemp, Warning, TEXT("[YiSan Step 2] Starting to load target level: %s"), *TargetLevelName.ToString());
    
    UGameplayStatics::OpenLevel(this, TargetLevelName);
}


void UYiSanGameInstance::OnTargetLevelReady()
{
    Step3_OnLevelLoaded();
}


// ==================== Step 3: 레벨 로드 완료 ====================

void UYiSanGameInstance::Step3_OnLevelLoaded()
{
    PRINTLOG(TEXT("메모리 로딩"));
    bLevelLoaded = true;
    // 이제 리소스(텍스처, 셰이더 등)가 완전히 준비될 때까지 대기
    Step4_CheckResources();
}

// ==================== Step 4: 리소스 스트리밍 체크 ====================

void UYiSanGameInstance::Step4_CheckResources()
{
    PRINTLOG(TEXT("리소스 체크중"));
    if (UWorld* World = GetWorld())
    {
        GetTimerManager().SetTimer(ResourceCheckTimer,this,&UYiSanGameInstance::PeriodicResourceCheck,0.1f,true);
    }
}

void UYiSanGameInstance::PeriodicResourceCheck()
{
    bool bTextureReady = CheckTextureStreaming();
    bool bShaderReady = CheckShaderCompilation();
    bool worldPartitionReady = CheckWorldPartition();
    if (bTextureReady && bShaderReady && worldPartitionReady)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(ResourceCheckTimer);
        }
        Step5_TransitionToTarget();
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
            PRINTLOG(TEXT(" 쉐이더 %d 다 됐습니다"), RemainingJobs);
            return false;
        }
    }
#endif
    // 패키징된 빌드에서는 항상 true 되어있어야 한다
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
                PRINTLOG(TEXT("월드 파티션쪽도 거의다 끝났슴다"));
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
        World->GetTimerManager().SetTimer(FinalDelayTimer,this,&UYiSanGameInstance::FinalHideLoadingScreen,0.3,false); 
    }
}
void UYiSanGameInstance::FinalHideLoadingScreen()
{
    HideLoadingScreen();
    PRINTLOG(TEXT("레벨 트랜지션 끝났습니다"));
    // 빠져 나옵니다
    bIsLoadingLevel = false;
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
                UE_LOG(LogTemp, Warning, TEXT("[YiSan] Loading screen shown"));
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
        UE_LOG(LogTemp, Warning, TEXT("[YiSan] Loading screen hidden"));
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
    FString netIDString = netID->ToString();
    UE_LOG(LogTemp, Warning, TEXT("서브시스템 : %s"), *subsysName.ToString());
    /*UE_LOG(LogTemp, Warning, TEXT("netID : %s"), *netIDString);
    sessionInterface->CreateSession(*netID, FName(displayName), sessionSettings);*/
    sessionInterface->CreateSession(0, FName(displayName), sessionSettings);


}

void UYiSanGameInstance::OnCreateSessionComplete(FName sessionName, bool success)
{
    if (success)
    {
        
        UE_LOG(LogTemp, Warning, TEXT("세션 : %s 성공"), *sessionName.ToString());
        GetWorld()->ServerTravel(TEXT("/Game/CustomContents/Maps/StartLevel?listen"));
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