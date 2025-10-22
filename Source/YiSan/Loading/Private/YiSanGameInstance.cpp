#include "YiSanGameInstance.h"
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

// 로딩 UI
void UYiSanGameInstance::ShowLoadingUI(TSubclassOf<UUserWidget> InLoadingWidgetClass)
{
    if (!GEngine)
    {
        UE_LOG(LogTemp, Error, TEXT("[로딩UI] GEngine이 존재하지 않습니다."));
        return;
    }

    if (LoadingWidgetObject.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[로딩UI] 이미 로딩 UI가 표시되어 있습니다."));
        return;
    }

    if (!InLoadingWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[로딩UI] 로딩 위젯 클래스가 설정되지 않았습니다."));
        return;
    }

    UUserWidget* Created = CreateWidget<UUserWidget>(this, InLoadingWidgetClass);
    if (!Created)
    {
        UE_LOG(LogTemp, Error, TEXT("[로딩UI] 위젯 생성 실패."));
        return;
    }

    LoadingWidgetObject = Created;

    // 뷰포트에 슬레이트 위젯으로 추가하려 시도
    if (GEngine->GameViewport)
    {
        TSharedRef<SWidget> SlateWidget = Created->TakeWidget();
        GEngine->GameViewport->AddViewportWidgetContent(SlateWidget, 1);
        LoadingWidgetHolder = SlateWidget;
        UE_LOG(LogTemp, Display, TEXT("[로딩UI] 뷰포트에 로딩 UI 추가 완료."));
    }
    else
    {
        // 백업: AddToViewport
        Created->AddToViewport(9999);
        UE_LOG(LogTemp, Display, TEXT("[로딩UI] GameViewport가 없어 AddToViewport로 추가했습니다."));
    }
}


// 헬퍼: 로딩 UI 제거
void UYiSanGameInstance::HideLoadingUI()
{
    UE_LOG(LogTemp, Display, TEXT("[로딩UI] 로딩 UI 제거 시도."));

    if (LoadingWidgetObject.IsValid())
    {
        UUserWidget* W = LoadingWidgetObject.Get();
        if (W)
        {
            W->RemoveFromParent();
        }
        LoadingWidgetObject = nullptr;
    }
    
    // Slate 위젯 홀더 제거
    if (GEngine && GEngine->GameViewport)
    {
        // TSharedPtr 타입이라고 가정하고 IsValid()로 유효성 검사
        if (LoadingWidgetHolder.IsValid())
        {
            // TSharedPtr는 TSharedRef로 안전하게 변환하여 제거 함수에 전달 가능
            GEngine->GameViewport->RemoveViewportWidgetContent(LoadingWidgetHolder.ToSharedRef());
            
            // 뷰포트에서 제거했으니 포인터 초기화
            LoadingWidgetHolder.Reset();
        }
    }
    UE_LOG(LogTemp, Display, TEXT("[로딩UI] 로딩 UI 제거 완료."));
}

// Step2: 타겟 레벨 로드 시작 
void UYiSanGameInstance::Step2_StartLoadingTargetLevel()
{
    UE_LOG(LogTemp, Warning, TEXT("[스텝2] 타겟 레벨 로드 시작: %s"), *TargetLevelName.ToString());

    // 중복 바인딩 방지
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UYiSanGameInstance::OnPostLoadMap);

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[스텝2] GetWorld()가 null입니다. 로드 중단."));
        return;
    }

    // 로딩 UI 띄우기 (사전에 LoadingWidgetClass를 할당해야 함)
    if (LoadingWidgetClass)
    {
        ShowLoadingUI(LoadingWidgetClass);
    }

    // 서버 모드(리슨서버 또는 데디케이티드)인지 확인
    ENetMode NetMode = World->GetNetMode();
    bool bIsServerMode = (NetMode == NM_ListenServer) || (NetMode == NM_DedicatedServer);

    const FString TargetLevelString = TargetLevelName.ToString();
    const bool bUseSeamless = true;

    if (bIsServerMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("[스텝2] 서버 모드이므로 ServerTravel로 이동합니다: %s, Seamless: %s"), 
            *TargetLevelString, bUseSeamless ? TEXT("ON") : TEXT("OFF"));
    
        // ServerTravel은 listen 서버에서 사용해야 클라이언트가 따라옴
        FString TravelURL = FString::Printf(TEXT("%s?listen"), *TargetLevelString);
    
        // bAbsolute를 true로 설정하여 전체 경로가 아닌 맵 이름으로 트래블.
        // SeamlessTravel이 실패하면 Non-Seamless Travel이 발생할 수 있음.
        World->ServerTravel(TravelURL, bUseSeamless); 
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[스텝2] 클라이언트/싱글플레이 모드이므로 OpenLevel 사용: %s"), *TargetLevelString);
        UGameplayStatics::OpenLevel(this, TargetLevelName);
    }
}


// OnPostLoadMap: 맵 로드 직후 호출되는 콜백 뷰포트 위젯 재생성(서버 트래블 등으로 제거된 경우 대비) 스트리밍/레벨인스턴스 준비 상태 폴링 시작
void UYiSanGameInstance::OnPostLoadMap(UWorld* LoadedWorld)
{
    // 델리게이트 해제(중복 방지)
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

    if (!LoadedWorld)
    {
        UE_LOG(LogTemp, Error, TEXT("[OnPostLoadMap] LoadedWorld가 null입니다."));
        return;
    }

    UE_LOG(LogTemp, Display, TEXT("[OnPostLoadMap] 맵 로드 완료: %s"), *LoadedWorld->GetName());

    // 로딩 UI가 없다면 재생성 (ServerTravel 등으로 인해 제거되었을 수 있음)
    if (!LoadingWidgetObject.IsValid() && !LoadingWidgetHolder.IsValid() && LoadingWidgetClass)
    {
        UE_LOG(LogTemp, Display, TEXT("[OnPostLoadMap] 로딩 UI가 없어 재생성합니다."));
        ShowLoadingUI(LoadingWidgetClass);
    }

    // 스트리밍 / 인스턴스 준비 상태 폴링 시작
    const float PollInterval = 0.25f;
    LoadedWorld->GetTimerManager().SetTimer(PollingStreamingTimerHandle, this, &UYiSanGameInstance::Poll_StreamingAndInstancesReady, PollInterval, true);
}

 
// Poll_StreamingAndInstancesReady: 주기적으로 호출되어 WorldPartition 스트리밍 완료 여부 텍스처 스트리밍 진행률 레벨 인스턴스의 실제 로드 상태 를 검사하고 모두 준비되면 UI 제거 및 다음 단계 호출
void UYiSanGameInstance::Poll_StreamingAndInstancesReady()
{
    UWorld* World = GetWorld();
    if (!World) return;

    bool bWorldPartitionReady = false;
    bool bTextureReady = false;
    bool bLevelInstancesReady = false;

    // WorldPartition 체크 
    if (UWorldPartitionSubsystem* WPS = World->GetSubsystem<UWorldPartitionSubsystem>())
    {
        // 한국어 주석: IsStreamingCompleted가 true이면 스트리밍(셀 로딩)이 완료된 상태
        bWorldPartitionReady = WPS->IsStreamingCompleted();
        UE_LOG(LogTemp, Display, TEXT("[폴링] WorldPartition 완료 여부: %s"), bWorldPartitionReady ? TEXT("완료") : TEXT("진행중"));
    }
    else
    {
        bWorldPartitionReady = true;
        UE_LOG(LogTemp, Display, TEXT("[폴링] WorldPartitionSubsystem가 없어 체크를 스킵합니다."));
    }

    //텍스처 스트리밍 체크
    {
        IStreamingManager& StreamingManager = IStreamingManager::Get();
        const float RequestSeconds = 0.1f;
        StreamingManager.UpdateResourceStreaming(RequestSeconds, true);
        float StreamingPercentage = StreamingManager.StreamAllResources(RequestSeconds);
        bTextureReady = FMath::IsFinite(StreamingPercentage) && (StreamingPercentage >= 0.99f || FMath::IsNearlyZero(StreamingPercentage));

        if (!FMath::IsFinite(StreamingPercentage) || StreamingPercentage < 0.0f)
        {
            // 통계 읽기 실패 또는 음수 값
            UE_LOG(LogTemp, Warning, TEXT("[폴링] 텍스처 스트리밍 진행률 읽기 실패 또는 음수값: %.3f"), StreamingPercentage);
            bTextureReady = false;
        }
        // 로드 완료 상태를 0.0f 또는 0.99f 이상으로 간주
        else if (FMath::IsNearlyZero(StreamingPercentage) || StreamingPercentage >= 0.99f) // 수정된 부분
        {
            UE_LOG(LogTemp, Display, TEXT("[폴링] 텍스처 스트리밍 진행률: %.2f%%"), StreamingPercentage * 100.0f);
            bTextureReady = true; 
        }
        else
        {
            // 0.0f 초과, 0.99f 미만은 진행 중
            UE_LOG(LogTemp, Display, TEXT("[폴링] 텍스처 스트리밍 진행률: %.2f%%"), StreamingPercentage * 100.0f);
            bTextureReady = false;
        }
    }

    //레벨 인스턴스 체크
    float LevelInstanceProgress = 0.0f;
    if (ULevelInstanceSubsystem* LevelInstSub = World->GetSubsystem<ULevelInstanceSubsystem>())
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsOfClass(World, ALevelInstance::StaticClass(), Found);

        if (Found.Num() == 0)
        {
            LevelInstanceProgress = 1.0f;
            bLevelInstancesReady = true;
            UE_LOG(LogTemp, Display, TEXT("[폴링] 레벨 인스턴스가 없습니다. (체크 패스)"));
        }
        else
        {
            int32 ReadyCount = 0;
            for (AActor* Actor : Found)
            {
                if (ALevelInstance* LI = Cast<ALevelInstance>(Actor))
                {
                    ULevel* SubLevel = LI->GetLevel(); 
                    if (SubLevel && SubLevel->Actors.Num() > 0)
                    {
                        ReadyCount++;
                    }
                }
            }
            LevelInstanceProgress = (float)ReadyCount / Found.Num();
            bLevelInstancesReady = (ReadyCount == Found.Num());
        }
    }
    else
    {
        bLevelInstancesReady = true; LevelInstanceProgress = 1.0f;
    }

    float WorldPartitionProgress = bWorldPartitionReady ? 1.0f : 0.0f;
    float TextureProgress = FMath::Clamp(0.5, 0.0f, 0.99f) + (bTextureReady ? 0.01f : 0.0f); // 0.0 ~ 1.0에 맞춤
    
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
    
    // 모두 준비되었을 때 처리 
    if (bWorldPartitionReady && bTextureReady && bLevelInstancesReady)
    {
        UE_LOG(LogTemp, Display, TEXT("[폴링] 모든 준비 완료: WorldPartition(%s), Texture(%s), LevelInstance(%s)"),
            bWorldPartitionReady ? TEXT("OK") : TEXT("NO"),
            bTextureReady ? TEXT("OK") : TEXT("NO"),
            bLevelInstancesReady ? TEXT("OK") : TEXT("NO"));

        // 타이머 정지
        World->GetTimerManager().ClearTimer(PollingStreamingTimerHandle);

        // 로딩 UI 제거
        HideLoadingUI();

        // 다음 단계 호출
        Step5_TransitionToTarget();
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("[폴링] 아직 준비되지 않았습니다. 계속 대기합니다."));
    }
}

// CheckWorldPartition
bool UYiSanGameInstance::CheckWorldPartition()
{
    UWorld* World = GetWorld();
    if (!World) return true;

    if (UWorldPartitionSubsystem* WPS = World->GetSubsystem<UWorldPartitionSubsystem>())
    {
        bool bStreamingCompleted = WPS->IsStreamingCompleted();
        if (!bStreamingCompleted)
        {
            UE_LOG(LogTemp, Display, TEXT("[CheckWorldPartition] 아직 스트리밍 중입니다."));
            return false;
        }
        else
        {
            UE_LOG(LogTemp, Display, TEXT("[CheckWorldPartition] 스트리밍 완료."));
            return true;
        }
    }

    // WorldPartitionSubsystem이 없으면 체크를 패스
    return true;
}


// 기존에 있던 CheckTextureStreaming 함수(호출 필요 시 사용)

bool UYiSanGameInstance::CheckTextureStreaming()
{
    IStreamingManager& StreamingManager = IStreamingManager::Get();
    const float RequestSeconds = 0.1f;
    StreamingManager.UpdateResourceStreaming(RequestSeconds, true);
    float StreamingPercentage = StreamingManager.StreamAllResources(RequestSeconds);

    UE_LOG(LogTemp, Display, TEXT("[CheckTextureStreaming] 진행률: %.2f%%"), StreamingPercentage * 100.0f);

    if (!FMath::IsFinite(StreamingPercentage) || StreamingPercentage < 0.0f) return false;
    return StreamingPercentage >= 0.99f;
}

// Step5_TransitionToTarget 더미
void UYiSanGameInstance::Step5_TransitionToTarget()
{
    UE_LOG(LogTemp, Display, TEXT("[스텝5] 타겟으로 전환 중..."));
}