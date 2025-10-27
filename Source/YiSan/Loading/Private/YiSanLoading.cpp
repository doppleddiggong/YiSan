// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#include "YiSanLoading.h"
#include "LoadingScreenWidget.h"
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

void UYiSanLoading::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Display, TEXT("[YiSanLoading] 서브시스템 초기화"));
	
	// PreLoadMap 델리게이트 바인딩 (맵 전환 시작 시 호출)
	PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UYiSanLoading::Step1_OnPreLoadMap);
	
	// PostLoadMapWithWorld 델리게이트 바인딩 (맵 로드 완료 시 호출)
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UYiSanLoading::Step2_OnPostLoadMap);
	
	UE_LOG(LogTemp, Display, TEXT("[YiSanLoading] 델리게이트 바인딩 완료 - 맵 전환 자동 감지 활성화"));
}

void UYiSanLoading::Deinitialize()
{
	// 델리게이트 정리
	if (PreLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
	}
	
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
	}
	
	// 로딩 UI 정리
	HideLoadingUI();
	
	UE_LOG(LogTemp, Display, TEXT("[YiSanLoading] 서브시스템 정리 완료"));
	
	Super::Deinitialize();
}

void UYiSanLoading::Step1_OnPreLoadMap(const FString& MapName)
{
	// 맵 전환이 시작될 때 자동으로 호출됨
	UE_LOG(LogTemp, Warning, TEXT("[스텝1 - 자동] 맵 전환 감지: %s"), *MapName);
	
	// StartMap이나 TransitionMap 같은 것은 무시
	if (MapName.Contains(TEXT("StartMap")) || 
	    MapName.Contains(TEXT("Transition")) || 
	    MapName.Contains(TEXT("Entry")))
	{
		UE_LOG(LogTemp, Display, TEXT("[스텝1] 시작 맵으로 전환 중 - 로딩 UI 표시 안 함"));
		return;
	}
	
	// 로딩 위젯 클래스가 설정되어 있으면 UI 표시
	if (LoadingWidgetClass)
	{
		UE_LOG(LogTemp, Display, TEXT("[스텝1] 로딩 UI 표시 시작"));
		ShowLoadingUI(LoadingWidgetClass);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[스텝1] LoadingWidgetClass가 설정되지 않음 - UI 표시 안 함"));
	}
}

void UYiSanLoading::Step2_OnPostLoadMap(UWorld* LoadedWorld)
{
	if (!LoadedWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("[스텝2] LoadedWorld가 null임."));
		return;
	}

	// === 로컬 월드인지 확인 (멀티플레이어 대응) ===
	bool bIsLocalWorld = false;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULocalPlayer* LP = GI->GetFirstGamePlayer())
		{
			if (LP->GetWorld() == LoadedWorld)
			{
				bIsLocalWorld = true;
			}
		}
	}

	if (!bIsLocalWorld)
	{
		UE_LOG(LogTemp, Warning, TEXT("[스텝2] 로컬 월드가 아님, 폴링 스킵 (다른 플레이어의 월드)"));
		return;
	}
	// === 로컬 월드 체크 끝 ===

	UE_LOG(LogTemp, Display, TEXT("[스텝2 - 로컬] 맵 로드 완료: %s"), *LoadedWorld->GetName());

	// StartMap이나 TransitionMap이면 로딩 추적 안 함
	FString WorldName = LoadedWorld->GetName();
	if (WorldName.Contains(TEXT("StartMap")) || 
	    WorldName.Contains(TEXT("Transition")) || 
	    WorldName.Contains(TEXT("Entry")))
	{
		UE_LOG(LogTemp, Display, TEXT("[스텝2] 시작 맵 - 로딩 추적 안 함"));
		HideLoadingUI();
		return;
	}

	// 로딩 UI가 없다면 재생성 (ServerTravel 등으로 제거되었을 수 있음)
	if (!LoadingWidgetObject.IsValid() && !LoadingWidgetHolder.IsValid() && LoadingWidgetClass)
	{
		UE_LOG(LogTemp, Display, TEXT("[스텝2 - 로컬] 로딩 UI가 없어 재생성함."));
		ShowLoadingUI(LoadingWidgetClass);
	}
	
	// 타임아웃 체크 시작 시간 기록
	ResourceCheckStartTime = LoadedWorld->GetTimeSeconds();
	TextureStreamingStartTime = ResourceCheckStartTime;
	bInitialTextureStreamingComplete = false;

	// 텍스처 스트리밍 강제 시작
	IStreamingManager::Get().AddLevel(LoadedWorld->PersistentLevel);
	IStreamingManager::Get().NotifyLevelChange();
    
	UE_LOG(LogTemp, Warning, TEXT("[스텝2 - 로컬] 텍스처 스트리밍 강제 시작 완료"));

	// 스트리밍 폴링 시작
	const float PollInterval = 0.1f;
	LoadedWorld->GetTimerManager().SetTimer(
		PollingStreamingTimerHandle, 
		this, 
		&UYiSanLoading::Poll_StreamingAndInstancesReady, 
		PollInterval, 
		true
	);
	
	UE_LOG(LogTemp, Display, TEXT("[스텝2 - 로컬] 폴링 타이머 시작 (%.2f초 간격)"), PollInterval);
}

void UYiSanLoading::Step3_TransitionToTarget()
{
	UE_LOG(LogTemp, Display, TEXT("[스텝3 - 로컬] 타겟으로 전환 완료함. 플레이어 입력 활성화."));
    
	// 플레이어 입력 활성화 (로컬 플레이어만)
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (PC->IsLocalController())
			{
				PC->SetInputMode(FInputModeGameOnly());
				PC->bShowMouseCursor = false;
				UE_LOG(LogTemp, Display, TEXT("[스텝3 - 로컬] 입력 모드 변경 완료"));
			}
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

    // 1. WorldPartition 체크
    if (UWorldPartitionSubsystem* WPS = World->GetSubsystem<UWorldPartitionSubsystem>())
    {
        bWorldPartitionReady = WPS->IsStreamingCompleted();
    }
    else
    {
        bWorldPartitionReady = true;
    }

    // 2. 텍스처 스트리밍 체크
    {
        IStreamingManager& StreamingManager = IStreamingManager::Get();
        
        float CurrentTime = World->GetTimeSeconds();
        float ElapsedTime = CurrentTime - TextureStreamingStartTime;
        
        int32 NumStreamingTextures = StreamingManager.GetNumWantingResources();
        
        if (NumStreamingTextures == 0)
        {
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
            const float RequestSeconds = 0.1f;
            float RawPercentage = StreamingManager.StreamAllResources(RequestSeconds);
            
            if (!FMath::IsFinite(RawPercentage) || RawPercentage < 0.0f)
            {
                float TimeBasedProgress = FMath::Clamp((ElapsedTime - 2.0f) / 10.0f, 0.0f, 0.5f);
                StreamingPercentage = TimeBasedProgress;
                
                if (ElapsedTime >= 5.0f && FMath::Fmod(ElapsedTime, 5.0f) < 0.2f)
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
                StreamingPercentage = RawPercentage;
                bTextureReady = false;
                
                if (FMath::Fmod(ElapsedTime, 5.0f) < 0.2f)
                {
                    UE_LOG(LogTemp, Display, TEXT("[폴링] 텍스처 스트리밍 진행 중 (진행률: %.1f%%, 대기: %d개)"), 
                        StreamingPercentage * 100.0f, NumStreamingTextures);
                }
            }
        }
        
        // 타임아웃 처리
        if (!bTextureReady && ElapsedTime > 60.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("[폴링] 텍스처 스트리밍 타임아웃 (60초 초과). 강제 완료 처리함."));
            StreamingPercentage = 1.0f;
            bTextureReady = true;
        }
        
        // 진행 없음 감지
        static float LastPercentage = -1.0f;
        static float StuckTime = 0.0f;
        
        if (FMath::Abs(StreamingPercentage - LastPercentage) < 0.01f)
        {
            StuckTime += 0.1f;
            
            if (StuckTime > 10.0f && !bTextureReady)
            {
                UE_LOG(LogTemp, Warning, TEXT("[폴링] 텍스처 스트리밍 정체 감지. 진행 강제함."));
                StreamingPercentage = FMath::Min(StreamingPercentage + 0.2f, 1.0f);
                StuckTime = 0.0f;
            }
        }
        else
        {
            StuckTime = 0.0f;
        }
        LastPercentage = StreamingPercentage;
    }

    // 3. 레벨 인스턴스 체크
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
    
    // 가중치: 텍스처가 가장 중요 (70%)
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
    UpdateLoadingUIProgress(OverallProgress, StatusText, WorldPartitionProgress, TextureProgress, LevelInstanceProgress);
    
    // 5. 모든 준비가 완료되었는지 확인
    if (bWorldPartitionReady && bTextureReady && bLevelInstancesReady)
    {
        UE_LOG(LogTemp, Display, TEXT("[폴링] ===== 모든 준비 완료 ====="));
        UE_LOG(LogTemp, Display, TEXT("  WorldPartition: OK"));
        UE_LOG(LogTemp, Display, TEXT("  Texture: %.1f%% (완료)"), TextureProgress * 100.0f);
        UE_LOG(LogTemp, Display, TEXT("  LevelInstance: %.0f%% (완료)"), LevelInstanceProgress * 100.0f);
        UE_LOG(LogTemp, Display, TEXT("  총 소요 시간: %.2f초"), World->GetTimeSeconds() - ResourceCheckStartTime);

        // 타이머 정지
        World->GetTimerManager().ClearTimer(PollingStreamingTimerHandle);

        // 약간의 딜레이 후 로딩 UI 제거
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

	// 뷰포트에 슬레이트 위젯으로 추가
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
    
	// Slate 위젯 홀더 제거
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

void UYiSanLoading::UpdateLoadingUIProgress(float ProgressPercentage, const FText& StatusText, 
	float WorldPartitionProgress, float TextureProgress, float LevelInstanceProgress)
{
	// 10% 단위로만 로그 출력
	static int32 LastLoggedPercent = -1;
	int32 CurrentPercent = FMath::FloorToInt(ProgressPercentage * 10.0f) * 10;
    
	if (CurrentPercent != LastLoggedPercent)
	{
		UE_LOG(LogTemp, Display, TEXT("[UI업데이트] 진행률: %d%%, 상태: %s"), CurrentPercent, *StatusText.ToString());
		UE_LOG(LogTemp, Display, TEXT("  - WorldPartition: %.0f%%, Texture: %.0f%%, LevelInstance: %.0f%%"), 
			WorldPartitionProgress * 100.0f, TextureProgress * 100.0f, LevelInstanceProgress * 100.0f);
		LastLoggedPercent = CurrentPercent;
	}

	// 실제 UI 위젯 업데이트
	if (LoadingWidgetObject.IsValid())
	{
		if (ULoadingScreenWidget* LoadingWidget = Cast<ULoadingScreenWidget>(LoadingWidgetObject.Get()))
		{
			// 전체 진행률 업데이트
			LoadingWidget->UpdateOverallProgress(ProgressPercentage, StatusText);
			
			// 개별 컴포넌트 진행률 업데이트
			LoadingWidget->UpdateComponentProgress(WorldPartitionProgress, TextureProgress, LevelInstanceProgress);
			
			// 컴포넌트별 상태 텍스트 업데이트
			FText WPStatus = WorldPartitionProgress >= 1.0f ? 
				FText::FromString(TEXT("✓ 완료")) : 
				FText::FromString(FString::Printf(TEXT("진행 중 (%.0f%%)"), WorldPartitionProgress * 100.0f));
			
			FText TexStatus = TextureProgress >= 1.0f ? 
				FText::FromString(TEXT("✓ 완료")) : 
				FText::FromString(FString::Printf(TEXT("로딩 중 (%.0f%%)"), TextureProgress * 100.0f));
			
			FText LIStatus = LevelInstanceProgress >= 1.0f ? 
				FText::FromString(TEXT("✓ 완료")) : 
				FText::FromString(FString::Printf(TEXT("초기화 중 (%.0f%%)"), LevelInstanceProgress * 100.0f));
			
			LoadingWidget->UpdateComponentStatus(WPStatus, TexStatus, LIStatus);
		}
	}
}