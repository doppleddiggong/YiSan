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
#include "Engine/StreamableManager.h" 
#include "ContentStreaming.h"
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
    
    // UWorld* World = GetWorld();
    // if (!World)
    // {
    //     UE_LOG(LogTemp, Error, TEXT("[스텝1] GetWorld()가 null임. 로드 중단함."));
    //     return;
    // }
    //
    // // 로딩 UI 띄움
    // if (LoadingWidgetClass)
    // {
    //     ShowLoadingUI(LoadingWidgetClass);
    // }

    // 서버 모드(리슨서버/데디케이티드)인지 확인함
    // ENetMode NetMode = World->GetNetMode();
    // bool bIsServerMode = (NetMode == NM_ListenServer) || (NetMode == NM_DedicatedServer);

    // const FString TargetLevelString = TargetLevelName.ToString();
    // const bool bUseSeamless = true;
    GetWorld()->ServerTravel("/Game/CustomContents/Maps/MainMap_WP");

    // 중복 바인딩 방지함
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
    // 맵 로드 완료 시 Step2_OnPostLoadMap 함수를 호출하도록 바인딩함
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UYiSanGameInstance::Step2_OnPostLoadMap);
    
    // if (bIsServerMode)
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("[스텝1] 서버 모드이므로 ServerTravel로 이동함: %s, Seamless: %s"), 
    //         *TargetLevelString, bUseSeamless ? TEXT("ON") : TEXT("OFF"));
    //
    //     FString TravelURL = FString::Printf(TEXT("%s"), *TargetLevelString);
    //     
    //     // ServerTravel 실행 (RPC 통신용이므로 수정하지 않음)
    //     World->ServerTravel(TravelURL, bUseSeamless); 
    // }
    // else
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("[스텝1] 클라이언트/싱글플레이 모드이므로 OpenLevel 사용함: %s"), *TargetLevelString);
    //     UGameplayStatics::OpenLevel(this, TargetLevelName);
    // }
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
	
    // 타임아웃 체크 시작 시간 기록
    ResourceCheckStartTime = LoadedWorld->GetTimeSeconds();
    TextureStreamingStartTime = ResourceCheckStartTime; // 텍스처 스트리밍 시작 시간 기록
    bInitialTextureStreamingComplete = false; // 초기화

    // **수정: 텍스처 스트리밍 강제 시작**
    IStreamingManager::Get().AddLevel(LoadedWorld->PersistentLevel);
    IStreamingManager::Get().NotifyLevelChange();
    
    UE_LOG(LogTemp, Warning, TEXT("[스텝2] 텍스처 스트리밍 강제 시작 완료"));

    // 스트리밍 / 인스턴스 준비 상태 폴링 시작함
    const float PollInterval = 0.1f; // 0.25초 → 0.1초로 단축 (더 빠른 반응)
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
	
    float StreamingPercentage = 0.0f;
    float LevelInstanceProgress = 0.0f;

    // 1. WorldPartition 체크함
    if (UWorldPartitionSubsystem* WPS = World->GetSubsystem<UWorldPartitionSubsystem>())
    {
        bWorldPartitionReady = WPS->IsStreamingCompleted();
    }
    else
    {
        bWorldPartitionReady = true;
    }

    // 2. **개선된 텍스처 스트리밍 체크**
    {
        IStreamingManager& StreamingManager = IStreamingManager::Get();
        
        // 현재 시간
        float CurrentTime = World->GetTimeSeconds();
        float ElapsedTime = CurrentTime - TextureStreamingStartTime;
        
        // **방법 A: 대기중인 요청 수 확인 (가장 정확)**
        int32 NumStreamingTextures = StreamingManager.GetNumWantingResources();
        
        if (NumStreamingTextures == 0)
        {
            // 스트리밍할 텍스처가 없음 = 완료
            StreamingPercentage = 1.0f;
            bTextureReady = true;
            
            if (!bInitialTextureStreamingComplete)
            {
                UE_LOG(LogTemp, Display, TEXT("[폴링] 텍스처 스트리밍 완료 (대기 텍스처 0개, 소요 시간: %.2f초)"), ElapsedTime);
                bInitialTextureStreamingComplete = true;
            }
        }
        else
        {
            // **방법 B: StreamAllResources 사용하되 안전장치 추가**
            const float RequestSeconds = 0.1f;
            float RawPercentage = StreamingManager.StreamAllResources(RequestSeconds);
            
            // 진행률 계산 (음수/무한대 처리)
            if (!FMath::IsFinite(RawPercentage) || RawPercentage < 0.0f)
            {
                // DDC 빌드 중이거나 초기 상태 = 시간 기반 추정
                // 초기 10초는 0%에서 시작, 이후 서서히 증가
                float TimeBasedProgress = FMath::Clamp((ElapsedTime - 2.0f) / 10.0f, 0.0f, 0.5f);
                StreamingPercentage = TimeBasedProgress;
                
                if (ElapsedTime < 5.0f)
                {
                    // 처음 5초는 로그 생략 (DDC 초기화 중)
                }
                else
                {
                    UE_LOG(LogTemp, Display, TEXT("[폴링] 텍스처 DDC 빌드 중... (대기: %d개, 추정: %.1f%%, 경과: %.1f초)"), 
                        NumStreamingTextures, StreamingPercentage * 100.0f, ElapsedTime);
                }
                bTextureReady = false;
            }
            else if (RawPercentage >= 0.99f)
            {
                StreamingPercentage = 1.0f;
                bTextureReady = true;
                
                if (!bInitialTextureStreamingComplete)
                {
                    UE_LOG(LogTemp, Display, TEXT("[폴링] 텍스처 스트리밍 완료 (진행률: 100%%, 소요 시간: %.2f초)"), ElapsedTime);
                    bInitialTextureStreamingComplete = true;
                }
            }
            else
            {
                // 정상적인 진행 중 (0.0 ~ 0.99)
                StreamingPercentage = RawPercentage;
                bTextureReady = false;
                
                // 5초마다 한 번씩 로그 출력
                if (FMath::Fmod(ElapsedTime, 5.0f) < 0.2f)
                {
                    UE_LOG(LogTemp, Display, TEXT("[폴링] 텍스처 스트리밍 진행 중 (진행률: %.1f%%, 대기: %d개)"), 
                        StreamingPercentage * 100.0f, NumStreamingTextures);
                }
            }
        }
        
        // **방법 C: 타임아웃 처리 (60초 이상 걸리면 강제 완료)**
        if (!bTextureReady && ElapsedTime > 60.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("[폴링] 텍스처 스트리밍 타임아웃 (60초 초과). 강제 완료 처리함. 대기 텍스처: %d개"), NumStreamingTextures);
            UE_LOG(LogTemp, Warning, TEXT("[폴링] 일부 셰이더는 런타임에 컴파일됩니다. 게임 진행 가능."));
            StreamingPercentage = 1.0f;
            bTextureReady = true;
        }
        
        // **방법 D: 진행 없음 감지 (같은 진행률이 10초 이상 유지)**
        static float LastPercentage = -1.0f;
        static float StuckTime = 0.0f;
        
        if (FMath::Abs(StreamingPercentage - LastPercentage) < 0.01f)
        {
            StuckTime += 0.1f; // 폴링 주기만큼 증가
            
            if (StuckTime > 10.0f && !bTextureReady)
            {
                UE_LOG(LogTemp, Warning, TEXT("[폴링] 텍스처 스트리밍 정체 감지 (%.1f초 동안 %.1f%%). 진행 강제함."), 
                    StuckTime, StreamingPercentage * 100.0f);
                StreamingPercentage = FMath::Min(StreamingPercentage + 0.2f, 1.0f);
                StuckTime = 0.0f; // 리셋
            }
        }
        else
        {
            StuckTime = 0.0f; // 진행 있음, 리셋
        }
        LastPercentage = StreamingPercentage;
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
        }
        else
        {
            int32 ReadyCount = 0;
            for (AActor* Actor : Found)
            {
                if (ALevelInstance* LI = Cast<ALevelInstance>(Actor))
                {
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
        LevelInstanceProgress = 1.0f;
    }

    // 4. 진행률 및 상태 텍스트 계산
    float WorldPartitionProgress = bWorldPartitionReady ? 1.0f : 0.0f;
    float TextureProgress = FMath::Clamp(StreamingPercentage, 0.0f, 1.0f); 
    
    // **가중치 조정: 텍스처가 가장 중요 (70%)**
    float OverallProgress = 
        (WorldPartitionProgress * 0.15f) + 
        (TextureProgress * 0.70f) + 
        (LevelInstanceProgress * 0.15f);
    
    FText StatusText = FText::FromString(TEXT("맵 데이터 로딩 중..."));
    if (!bWorldPartitionReady)
        StatusText = FText::FromString(TEXT("월드 파티션 스트리밍 중..."));
    else if (!bTextureReady) 
        StatusText = FText::FromString(TEXT("텍스처 에셋 스트리밍 중... (DDC 빌드 포함)"));
    else if (!bLevelInstancesReady)
        StatusText = FText::FromString(TEXT("레벨 인스턴스 초기화 중..."));
    else if (OverallProgress >= 0.99f)
        StatusText = FText::FromString(TEXT("로딩 완료! 게임 시작 준비됨."));

    // UI 업데이트
    UpdateLoadingUIProgress(OverallProgress, StatusText);
    
    // 5. 모든 준비가 완료되었는지 확인
    if (bWorldPartitionReady && bTextureReady && bLevelInstancesReady)
    {
        UE_LOG(LogTemp, Display, TEXT("[폴링] ===== 모든 준비 완료 ====="));
        UE_LOG(LogTemp, Display, TEXT("  WorldPartition: OK"));
        UE_LOG(LogTemp, Display, TEXT("  Texture: %.1f%% (완료)"), TextureProgress * 100.0f);
        UE_LOG(LogTemp, Display, TEXT("  LevelInstance: %.0f%% (완료)"), LevelInstanceProgress * 100.0f);
        UE_LOG(LogTemp, Display, TEXT("  총 소요 시간: %.2f초"), World->GetTimeSeconds() - ResourceCheckStartTime);

        // 타이머 정지함
        World->GetTimerManager().ClearTimer(PollingStreamingTimerHandle);

        // 약간의 딜레이 후 로딩 UI 제거 (시각적 안정성)
        FTimerHandle DelayHandle;
        World->GetTimerManager().SetTimer(DelayHandle, [this]()
        {
            HideLoadingUI();
            Step3_TransitionToTarget();
        }, 0.5f, false);
    }
}

// UI 업데이트 함수
void UYiSanGameInstance::UpdateLoadingUIProgress(float ProgressPercentage, const FText& StatusText)
{
    // 10% 단위로만 로그 출력 (로그 스팸 방지)
    static int32 LastLoggedPercent = -1;
    int32 CurrentPercent = FMath::FloorToInt(ProgressPercentage * 10.0f) * 10; // 10% 단위
    
    if (CurrentPercent != LastLoggedPercent)
    {
        UE_LOG(LogTemp, Display, TEXT("[UI업데이트] 진행률: %d%%, 상태: %s"), CurrentPercent, *StatusText.ToString());
        LastLoggedPercent = CurrentPercent;
    }

    if (LoadingWidgetObject.IsValid())
    {
        UUserWidget* Widget = LoadingWidgetObject.Get();
        if (Widget)
        {
            // 실제 위젯 업데이트 (블루프린트에서 구현 필요)
            // 예: ILoadingWidgetInterface를 구현한 위젯이라면:
            // ILoadingWidgetInterface::Execute_UpdateProgress(Widget, ProgressPercentage, StatusText);
        }
    }
}




// Step 3: 타겟으로 전환
void UYiSanGameInstance::Step3_TransitionToTarget()
{
    UE_LOG(LogTemp, Display, TEXT("[스텝3] 타겟으로 전환 완료함. 플레이어 입력 활성화."));
    
    // 플레이어 입력 활성화
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PC->SetInputMode(FInputModeGameOnly());
            PC->bShowMouseCursor = false;
        }
    }
}

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
    if (!sessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface is invalid!"));
        return;
    }
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
        // Seamless Travel을 사용하여 클라이언트 연결 유지
        GetWorld()->ServerTravel(TEXT("/Game/CustomContents/Maps/StartLevel?listen"), true);
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