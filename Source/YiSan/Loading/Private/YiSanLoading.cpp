// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "YiSanLoading.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "TimerManager.h"
#include "UObject/WeakObjectPtr.h"
#include "LevelInstance/LevelInstanceSubsystem.h"
#include "Templates/SharedPointer.h" 
#include "Widgets/SWidget.h"
#include "GameFramework/PlayerController.h"
#include "ContentStreaming.h"

void UYiSanLoading::Step1_StartLoadingTargetLevel()
{
	UE_LOG(LogTemp, Warning, TEXT("[스텝1] 타겟 레벨 로드 시작함: %s"), *TargetLevelName.ToString());

	// 현재는 seamlessTravel 사용중
	const bool bUseSeamless = true;
	const FString MapPath = TEXT("/Game/CustomContents/Maps/MainMap_WP");
	//GetWorld()->SeamlessTravel("/Game/CustomContents/Maps/MainMap_WP", bUseSeamless);
	// 문제시 severtarvel 사용
	GetWorld()->ServerTravel("/Game/CustomContents/Maps/MainMap_WP",bUseSeamless);
	// 중복 바인딩 방지함
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	// 맵 로드 완료 시 Step2_OnPostLoadMap 함수를 호출하도록 바인딩함
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UYiSanLoading::Step2_OnPostLoadMap);
}

void UYiSanLoading::Step2_OnPostLoadMap(UWorld* LoadedWorld)
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
	LoadedWorld->GetTimerManager().SetTimer(PollingStreamingTimerHandle, this, &UYiSanLoading ::Poll_StreamingAndInstancesReady, PollInterval, true);
}

void UYiSanLoading::Step3_TransitionToTarget()
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

void UYiSanLoading::Poll_StreamingAndInstancesReady()
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

void UYiSanLoading::ShowLoadingUI(TSubclassOf<UUserWidget> InLoadingWidgetClass)
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

	UUserWidget* Created = CreateWidget<UUserWidget>(GetGameInstance(), InLoadingWidgetClass);
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

void UYiSanLoading::HideLoadingUI()
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

void UYiSanLoading::UpdateLoadingUIProgress(float ProgressPercentage, const FText& StatusText)
{
	// 10% 단위로만 로그 출력 (로그 스팸 방지)
	static int32 LastLoggedPercent = -1;
	int32 CurrentPercent = FMath::FloorToInt(ProgressPercentage * 10.0f) * 10; // 10% 단위
    
	if (CurrentPercent != LastLoggedPercent)
	{
		UE_LOG(LogTemp, Display, TEXT("[UI업데이트] 진행률: %d%%, 상태: %s"), CurrentPercent, *StatusText.ToString());
		LastLoggedPercent = CurrentPercent;
	}
}
