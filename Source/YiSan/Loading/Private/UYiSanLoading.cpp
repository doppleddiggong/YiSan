// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UYiSanLoading.h"

#include "FComponentHelper.h"
#include "GameLogging.h"
#include "UDialogManager.h"

#include "TimerManager.h"
#include "ContentStreaming.h"

#include "LevelInstance/LevelInstanceActor.h"
#include "LevelInstance/LevelInstanceSubsystem.h"
#include "GameFramework/PlayerController.h"

#include "WorldPartition/WorldPartitionSubsystem.h"

#include "Engine/World.h"
#include "Engine/Engine.h"

void UYiSanLoading::InitSystem(const FString& InURL, const bool bAbsolute)
{
	PRINTLOG(TEXT("로딩 레벨 매니저: Step1 로딩 시작 호출함"));

	TotalProgress = 0;
	
	bInitialTextureStreamingComplete = false;
	bWorldPartitionReady = false;
	bTextureReady = false;
	bLevelInstancesReady = false;
	StreamingPercentage = 0.0f;
	LevelInstanceProgress = 0.0f;

	DM = UDialogManager::Get(this);
	
	// 맵 로드 완료 시 Step2_OnPostLoadMap 함수를 호출하도록 바인딩함
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UYiSanLoading::PostLoadMapWithWorld);


	PRINTLOG( TEXT("[스텝1] 타겟 레벨 로드 시작함") );
	GetWorld()->ServerTravel(InURL, bAbsolute);
}

void UYiSanLoading::PostLoadMapWithWorld(UWorld* World)
{
	if (!World)
	{
		PRINTLOG( TEXT("[스텝2] LoadedWorld가 null임."));
		return;
	}

	PRINTLOG( TEXT("[스텝2] 맵 로드 완료: %s"), *World->GetName());
	DM->ShowToast( FString::Printf( TEXT("[스텝2] 맵 로드 완료: %s"), *World->GetName()));
	
	// 타임아웃 체크 시작 시간 기록
	ResourceCheckStartTime = World->GetTimeSeconds();
	bInitialTextureStreamingComplete = false; // 초기화

	// **수정: 텍스처 스트리밍 강제 시작**
	IStreamingManager::Get().AddLevel(World->PersistentLevel);
	IStreamingManager::Get().NotifyLevelChange();
    
	UE_LOG(LogTemp, Warning, TEXT("[스텝2] 텍스처 스트리밍 강제 시작 완료"));
	DM->ShowToast( FString::Printf( TEXT("[스텝2] 텍스처 스트리밍 강제 시작 완료")));

	// 스트리밍 / 인스턴스 준비 상태 폴링 시작함
	World->GetTimerManager().SetTimer(TimeHandlePool,
		this,
		&UYiSanLoading::Poll_StreamingAndInstancesReady, 0.1f, true);
}


void UYiSanLoading::Poll_StreamingAndInstancesReady()
{
	UWorld* World = GetWorld();
    if (!World) 
    {
        PRINTLOG( TEXT("[폴링] World가 null임. 폴링 중단."));
        return;
    }

    // WorldPartition 체크함
	if ( !bWorldPartitionReady )
	{
		if (auto WPS = World->GetSubsystem<UWorldPartitionSubsystem>())
			bWorldPartitionReady = WPS->IsStreamingCompleted();
		else
			bWorldPartitionReady = true;
	}

	this->Loading_Textures(World);
	this->Loading_LevelInstance(World);

    // 4. 진행률 및 상태 텍스트 계산
    const float WorldPartitionProgress = bWorldPartitionReady ? 1.0f : 0.0f;
    const float TextureProgress = FMath::Clamp(StreamingPercentage, 0.0f, 1.0f); 
    
    // **가중치 조정: 텍스처가 가장 중요 (70%)**
    TotalProgress = 
        (WorldPartitionProgress * 0.15f) + 
        (TextureProgress * 0.70f) + 
        (LevelInstanceProgress * 0.15f);
    
    // FText StatusText = FText::FromString(TEXT("맵 데이터 로딩 중..."));
    // if (!bWorldPartitionReady)
    //     StatusText = FText::FromString(TEXT("월드 파티션 스트리밍 중..."));
    // else if (!bTextureReady) 
    //     StatusText = FText::FromString(TEXT("텍스처 에셋 스트리밍 중... (DDC 빌드 포함)"));
    // else if (!bLevelInstancesReady)
    //     StatusText = FText::FromString(TEXT("레벨 인스턴스 초기화 중..."));
    // else if (TotalProgress >= 0.99f)
    //     StatusText = FText::FromString(TEXT("로딩 완료! 게임 시작 준비됨."));

    // UI 업데이트
    UpdateLoadingProgress();
    
    // 5. 모든 준비가 완료되었는지 확인
    if ( bWorldPartitionReady && bTextureReady && bLevelInstancesReady )
    {
    	PRINTLOG( TEXT("[폴링] ===== 모든 준비 완료 ====="));
    	PRINTLOG( TEXT("WorldPartition: OK"));
    	PRINTLOG( TEXT("Texture: %.1f%% (완료)"), TextureProgress * 100.0f);
    	PRINTLOG( TEXT("LevelInstance: %.0f%% (완료)"), LevelInstanceProgress * 100.0f);
    	PRINTLOG( TEXT("총 소요 시간: %.2f초"), World->GetTimeSeconds() - ResourceCheckStartTime);

    	CompleteProcess(World);
    }
}

void UYiSanLoading::CompleteProcess(const UWorld* World)
{
	// 타이머 정지함
	World->GetTimerManager().ClearTimer(TimeHandlePool);

	// 약간의 딜레이 후 로딩 UI 제거 (시각적 안정성)
	FTimerHandle DelayHandle;
	World->GetTimerManager().SetTimer(DelayHandle, [this]()
	{
		PRINTLOG( TEXT("[스텝3] 타겟으로 전환 완료함. 플레이어 입력 활성화."));
		DM->ShowToast( FString::Printf( TEXT("[스텝3] 타겟으로 전환 완료함. 플레이어 입력 활성화.")));

		// 플레이어 입력 활성화
		if (UWorld* World = GetWorld())
		{
			if (auto PC = World->GetFirstPlayerController())
			{
				PC->SetInputMode(FInputModeGameOnly());
				PC->bShowMouseCursor = false;

				//메인맵 들어갔음
				DM->ShowToast( FString::Printf( TEXT("GAME START!!!")));
			}
		}
	}, 0.1f, false);
}

// 2. **개선된 텍스처 스트리밍 체크**
void UYiSanLoading::Loading_Textures(const UWorld* World)
{
    IStreamingManager& StreamingManager = IStreamingManager::Get();
    
    // 현재 시간
    float CurrentTime = World->GetTimeSeconds();
    float ElapsedTime = CurrentTime - ResourceCheckStartTime;
    
    // **방법 A: 대기중인 요청 수 확인 (가장 정확)**
    int32 NumStreamingTextures = StreamingManager.GetNumWantingResources();
    
    if (NumStreamingTextures == 0)
    {
        // 스트리밍할 텍스처가 없음 = 완료
        StreamingPercentage = 1.0f;
        bTextureReady = true;
        
        if (!bInitialTextureStreamingComplete)
        {
            PRINTLOG( TEXT("[폴링] 텍스처 스트리밍 완료 (대기 텍스처 0개, 소요 시간: %.2f초)"), ElapsedTime);
            bInitialTextureStreamingComplete = true;
        }
    }
    else
    {
        // **방법 B: StreamAllResources 사용하되 안전장치 추가**
        float RawPercentage = StreamingManager.StreamAllResources(0.1f);
        
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
               PRINTLOG( TEXT("[폴링] 텍스처 DDC 빌드 중... (대기: %d개, 추정: %.1f%%, 경과: %.1f초)"), 
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
                PRINTLOG( TEXT("[폴링] 텍스처 스트리밍 완료 (진행률: 100%%, 소요 시간: %.2f초)"), ElapsedTime);
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
                PRINTLOG( TEXT("[폴링] 텍스처 스트리밍 진행 중 (진행률: %.1f%%, 대기: %d개)"), 
                    StreamingPercentage * 100.0f, NumStreamingTextures);
            }
        }
    }
    
    // **방법 C: 타임아웃 처리 (60초 이상 걸리면 강제 완료)**
    if (!bTextureReady && ElapsedTime > 60.0f)
    {
        PRINTLOG( TEXT("[폴링] 텍스처 스트리밍 타임아웃 (60초 초과). 강제 완료 처리함. 대기 텍스처: %d개"), NumStreamingTextures);
        PRINTLOG( TEXT("[폴링] 일부 셰이더는 런타임에 컴파일됩니다. 게임 진행 가능."));
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
            PRINTLOG( TEXT("[폴링] 텍스처 스트리밍 정체 감지 (%.1f초 동안 %.1f%%). 진행 강제함."), 
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

void UYiSanLoading::Loading_LevelInstance(UWorld* World)
{
	if (auto LevelInstSub = World->GetSubsystem<ULevelInstanceSubsystem>())
	{
		auto Found = FComponentHelper::GetAllOfClass<ALevelInstance>(World);
		if (Found.Num() == 0)
		{
			LevelInstanceProgress = 1.0f;
			bLevelInstancesReady = true;
		}
		else
		{
			int32 ReadyCount = 0;
			for ( auto LI : Found)
			{
				if (LI->GetLoadedLevel() != nullptr)
				{
					ReadyCount++;
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
}

void UYiSanLoading::UpdateLoadingProgress() const
{
	// 10% 단위로만 로그 출력 (로그 스팸 방지)
	int32 CurrentPercent = FMath::FloorToInt(TotalProgress * 10.0f) * 10; // 10% 단위
	PRINTLOG(TEXT("PROGRESS : %d"), CurrentPercent);
}
