#include "YiSanGameInstance.h"

#include "GameLogging.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/WeakObjectPtr.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "LevelInstance/LevelInstanceSubsystem.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "Templates/SharedPointer.h" 
#include "Widgets/SWidget.h"
#include "GameFramework/PlayerController.h"


#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

void UYiSanGameInstance::Init()
{
    Super::Init();
    UE_LOG(LogTemp, Log, TEXT("[YiSan GameInstance] Initialized"));

    this->InitSubsystem();
}

#pragma region OPTIMIZATION
// 로딩 UI 표시함
void UYiSanGameInstance::ShowLoadingUI(TSubclassOf<UUserWidget> InLoadingWidgetClass)
{
    if (!GEngine)
    {
        UE_LOG(LogTemp, Error, TEXT("[로딩UI] GEngine이 존재하지 않음."));
        return;
    }

    if (LoadingWidgetObject.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[로딩UI] 이미 로딩 UI가 표시되어 있음."));
        return;
    }

    if (!InLoadingWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[로딩UI] 로딩 위젯 클래스가 설정되지 않음."));
        return;
    }

    UUserWidget* Created = CreateWidget<UUserWidget>(this, InLoadingWidgetClass);
    if (!Created)
    {
        UE_LOG(LogTemp, Error, TEXT("[로딩UI] 위젯 생성 실패함."));
        return;
    }

    LoadingWidgetObject = Created;

    // 뷰포트에 슬레이트 위젯으로 추가 시도함
    if (GEngine->GameViewport)
    {
        TSharedRef<SWidget> SlateWidget = Created->TakeWidget();
        GEngine->GameViewport->AddViewportWidgetContent(SlateWidget, 1);
        LoadingWidgetHolder = SlateWidget;
        UE_LOG(LogTemp, Display, TEXT("[로딩UI] 뷰포트에 로딩 UI 추가 완료함."));
    }
    else
    {
        Created->AddToViewport(9999);
        UE_LOG(LogTemp, Display, TEXT("[로딩UI] GameViewport가 없어 AddToViewport로 추가함."));
    }
}


// 헬퍼: 로딩 UI 제거함
void UYiSanGameInstance::HideLoadingUI()
{
    UE_LOG(LogTemp, Display, TEXT("[로딩UI] 로딩 UI 제거 시도함."));

    if (LoadingWidgetObject.IsValid())
    {
        UUserWidget* W = LoadingWidgetObject.Get();
        if (W)
        {
            W->RemoveFromParent();
        }
        LoadingWidgetObject = nullptr;
    }
    
    // Slate 위젯 홀더 제거함
    if (GEngine && GEngine->GameViewport)
    {
        if (LoadingWidgetHolder.IsValid())
        {
            GEngine->GameViewport->RemoveViewportWidgetContent(LoadingWidgetHolder.ToSharedRef());
            LoadingWidgetHolder.Reset();
        }
    }
    UE_LOG(LogTemp, Display, TEXT("[로딩UI] 로딩 UI 제거 완료함."));
}

// Step 1: 타겟 레벨 로드 시작함
void UYiSanGameInstance::Step1_StartLoadingTargetLevel()
{
    UE_LOG(LogTemp, Warning, TEXT("[스텝1] 타겟 레벨 로드 시작함: %s"), *TargetLevelName.ToString());

    // 중복 바인딩 방지함
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
    // 맵 로드 완료 시 Step2_OnPostLoadMap 함수를 호출하도록 바인딩함
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UYiSanGameInstance::Step2_OnPostLoadMap);

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[스텝1] GetWorld()가 null임. 로드 중단함."));
        return;
    }

    // 로딩 UI 띄움
    if (LoadingWidgetClass)
    {
        ShowLoadingUI(LoadingWidgetClass);
    }

    // 서버 모드(리슨서버/데디케이티드)인지 확인함
    ENetMode NetMode = World->GetNetMode();
    bool bIsServerMode = (NetMode == NM_ListenServer) || (NetMode == NM_DedicatedServer);

    const FString TargetLevelString = TargetLevelName.ToString();
    const bool bUseSeamless = true;

    if (bIsServerMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("[스텝1] 서버 모드이므로 ServerTravel로 이동함: %s, Seamless: %s"), 
            *TargetLevelString, bUseSeamless ? TEXT("ON") : TEXT("OFF"));
    
        FString TravelURL = FString::Printf(TEXT("%s?listen"), *TargetLevelString);
        
        // ServerTravel 실행 (RPC 통신용이므로 수정하지 않음)
        World->ServerTravel(TravelURL, bUseSeamless); 
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[스텝1] 클라이언트/싱글플레이 모드이므로 OpenLevel 사용함: %s"), *TargetLevelString);
        UGameplayStatics::OpenLevel(this, TargetLevelName);
    }
}


// Step 2: 맵 로드 직후 호출되는 콜백임.
void UYiSanGameInstance::Step2_OnPostLoadMap(UWorld* LoadedWorld)
{
    // 델리게이트 해제함(중복 방지)
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

    if (!LoadedWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("[스텝2] LoadedWorld가 null임."));
        return;
    }

    UE_LOG(LogTemp, Display, TEXT("[스텝2] 맵 로드 완료: %s"), *LoadedWorld->GetName());

    // 로딩 UI가 없다면 재생성함 (ServerTravel 등으로 제거되었을 수 있음)
    if (!LoadingWidgetObject.IsValid() && !LoadingWidgetHolder.IsValid() && LoadingWidgetClass)
    {
        UE_LOG(LogTemp, Display, TEXT("[스텝2] 로딩 UI가 없어 재생성함."));
        ShowLoadingUI(LoadingWidgetClass);
    }
	
    // 타임아웃 체크 시작 시간 기록 (필요 시 사용)
    ResourceCheckStartTime = LoadedWorld->GetTimeSeconds();

    // 스트리밍 / 인스턴스 준비 상태 폴링 시작함
    const float PollInterval = 0.25f;
    LoadedWorld->GetTimerManager().SetTimer(PollingStreamingTimerHandle, this, &UYiSanGameInstance::Poll_StreamingAndInstancesReady, PollInterval, true);
}

 
// Poll_StreamingAndInstancesReady: 주기적으로 호출되어 스트리밍 상태를 검사함.
void UYiSanGameInstance::Poll_StreamingAndInstancesReady()
{
    UWorld* World = GetWorld();
    if (!World) 
    {
        UE_LOG(LogTemp, Error, TEXT("[폴링] World가 null임. 폴링 중단."));
        return;
    }

    bool bWorldPartitionReady = false;
    bool bTextureReady = false;
    bool bLevelInstancesReady = false;
	
    float StreamingPercentage = 0.0f; // (수정) 텍스처 진행률 변수를 상위 스코프로 이동
    float LevelInstanceProgress = 0.0f;

    // 1. WorldPartition 체크함
    if (UWorldPartitionSubsystem* WPS = World->GetSubsystem<UWorldPartitionSubsystem>())
    {
        bWorldPartitionReady = WPS->IsStreamingCompleted();
        // UE_LOG(LogTemp, Display, TEXT("[폴링] WorldPartition 완료 여부: %s"), bWorldPartitionReady ? TEXT("완료") : TEXT("진행중"));
    }
    else
    {
        bWorldPartitionReady = true; // WP 서브시스템이 없으면 체크 패스함
        // UE_LOG(LogTemp, Display, TEXT("[폴링] WorldPartitionSubsystem가 없어 체크 스킵함."));
    }

    // 2. 텍스처 스트리밍 체크함
    {
        IStreamingManager& StreamingManager = IStreamingManager::Get(); 
        const float RequestSeconds = 0.1f;
        
       // StreamingManager.UpdateResourceStreaming(RequestSeconds, true);
        
        StreamingPercentage = StreamingManager.StreamAllResources(RequestSeconds);
        
        // (수정) 텍스처 0% 처리 문제 수정: 99% 이상일 때만 완료로 간주함
        if (!FMath::IsFinite(StreamingPercentage) || StreamingPercentage < 0.0f)
        {
            // 통계 읽기 실패 또는 음수 값임
            UE_LOG(LogTemp, Warning, TEXT("[폴링] 텍스처 스트리밍 진행률 읽기 실패 또는 음수값임: %.3f"), StreamingPercentage);
            bTextureReady = false;
        }
        else if (StreamingPercentage >= 0.99f) // 99% 이상
        {
            // UE_LOG(LogTemp, Display, TEXT("[폴링] 텍스처 스트리밍 완료: %.2f%%"), StreamingPercentage * 100.0f);
            bTextureReady = true; 
        }
        else // 0.0 ~ 0.989... (진행 중)
        {
            // UE_LOG(LogTemp, Display, TEXT("[폴링] 텍스처 스트리밍 진행률: %.2f%%"), StreamingPercentage * 100.0f);
            if (StreamingPercentage < 0.01f)
            {
                PRINTLOG(TEXT("텍스쳐 스트리밍 진행율 %.2f%% "),StreamingPercentage * 100.0f);
            }
            bTextureReady = false;
        }
    }

    // 3. 레벨 인스턴스 체크함
    if (ULevelInstanceSubsystem* LevelInstSub = World->GetSubsystem<ULevelInstanceSubsystem>())
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsOfClass(World, ALevelInstance::StaticClass(), Found);

        if (Found.Num() == 0)
        {
            LevelInstanceProgress = 1.0f;
            bLevelInstancesReady = true;
            // UE_LOG(LogTemp, Display, TEXT("[폴링] 레벨 인스턴스가 없음. (체크 패스함)"));
        }
        else
        {
            int32 ReadyCount = 0;
            for (AActor* Actor : Found)
            {
                if (ALevelInstance* LI = Cast<ALevelInstance>(Actor))
                {
                    // 레벨 인스턴스 확인 방식: Actors.Num() 대신 IsLevelLoaded() 사용
                    if (LI->GetLoadedLevel() != nullptr)
                    {
                        ReadyCount++;
                    }
                }
            }
            LevelInstanceProgress = (Found.Num() > 0) ? ((float)ReadyCount / Found.Num()) : 1.0f;
            bLevelInstancesReady = (ReadyCount == Found.Num());
        }
    }
    else
    {
        bLevelInstancesReady = true; 
        LevelInstanceProgress = 1.0f; // 서브시스템 없으면 패스
    }

    // 4. 진행률 및 상태 텍스트 계산
    float WorldPartitionProgress = bWorldPartitionReady ? 1.0f : 0.0f;
    // 텍스처 진행률 계산 오류 수정: Clamp(0.5...) 대신 실제 진행률 Clamp
    float TextureProgress = FMath::Clamp(StreamingPercentage, 0.0f, 1.0f); 
    
    float OverallProgress = 
        (WorldPartitionProgress * 0.2f) + 
        (TextureProgress * 0.6f) + 
        (LevelInstanceProgress * 0.2f);
    
    FText StatusText = FText::FromString(TEXT("데이터 로딩 및 스트리밍 중..."));
    if (bWorldPartitionReady && !bTextureReady) 
        StatusText = FText::FromString(TEXT("에셋 스트리밍 최적화 중..."));
    else if (bWorldPartitionReady && bTextureReady && !bLevelInstancesReady)
        StatusText = FText::FromString(TEXT("레벨 인스턴스 초기화 중..."));
    else if (OverallProgress >= 0.99f)
        StatusText = FText::FromString(TEXT("로딩 완료. 잠시 후 게임 시작."));

    // 미사용 상태 텍스트 문제 해결: UI 업데이트 함수 호출
    UpdateLoadingUIProgress(OverallProgress, StatusText);
    
    // 5. 모든 준비가 완료되었는지 확인
    if (bWorldPartitionReady && bTextureReady && bLevelInstancesReady)
    {
        UE_LOG(LogTemp, Display, TEXT("[폴링] 모든 준비 완료: WorldPartition(%s), Texture(%.2f%%), LevelInstance(%d/%d)"),
            bWorldPartitionReady ? TEXT("OK") : TEXT("NO"),
            TextureProgress * 100.0f,
            (int32)(LevelInstanceProgress * 100.0f), 100);

        // 타이머 정지함
        World->GetTimerManager().ClearTimer(PollingStreamingTimerHandle);

        // 로딩 UI 제거함
        HideLoadingUI();

        // 다음 단계(Step3) 호출함
        Step3_TransitionToTarget();
    }
    else
    {
        // 아직 준비되지 않음 (타이머가 계속 실행됨)
        UE_LOG(LogTemp, Display, TEXT("[폴링] 아직 준비되지 않았습니다. 계속 대기합니다."));
    }
}

// (수정) 미사용 상태 텍스트 문제 해결을 위한 함수 구현
void UYiSanGameInstance::UpdateLoadingUIProgress(float ProgressPercentage, const FText& StatusText)
{
    // 로그로 현재 상태 출력
    UE_LOG(LogTemp, Display, TEXT("[UI업데이트] 진행률: %.1f%%, 상태: %s"), ProgressPercentage * 100.0f, *StatusText.ToString());

    // 실제 위젯이 유효한지 확인
    if (LoadingWidgetObject.IsValid())
    {
        UUserWidget* Widget = LoadingWidgetObject.Get();
        if (Widget)
        {
            // 참고: 여기에 실제 로딩 위젯의 함수를 호출해야 함.
            // 예: UMyLoadingWidget* MyWidget = Cast<UMyLoadingWidget>(Widget);
            // if (MyWidget)
            // {
            //     MyWidget->SetProgress(ProgressPercentage);
            //     MyWidget->SetStatusText(StatusText);
            // }
        }
    }
}


// Step 3: 타겟으로 전환 (더미 함수)
void UYiSanGameInstance::Step3_TransitionToTarget()
{
    UE_LOG(LogTemp, Display, TEXT("[스텝3] 타겟으로 전환 완료함."));
    
    // (예: 페이드인, 플레이어 입력 활성화 등)
}
#pragma endregion OPTIMIZATION



#pragma region ONLINE_SUB_SYSTEM

// ==================== Network 관리 ====================
void UYiSanGameInstance::InitSubsystem()
{
    // Super::Init();
    // UE_LOG(LogTemp, Log, TEXT("[YiSan GameInstance] Initialized"));
    
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
#pragma endregion ONLINE_SUB_SYSTEM