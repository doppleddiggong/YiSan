#include "LoadingLevelManager.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "WorldPartition/WorldPartitionSubsystem.h"

ALoadingLevelManager::ALoadingLevelManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALoadingLevelManager::BeginPlay()
{
	Super::BeginPlay();

	// [2025-10-20 Gemini-CLI] UYiSanGameInstance의 로딩 로직과 충돌하여 비활성화합니다.
	// 이 액터는 LoadingLevel에 배치되어 자체적으로 레벨 로딩을 시도하며,
	// 이는 GameInstance의 로딩 플로우를 방해하고 오류를 발생시킵니다.
	/*
	// 로딩 UI가 있다면 표시
	if (LoadingWidgetClass)
	{
		LoadingWidget = CreateWidget<UUserWidget>(GetWorld(), LoadingWidgetClass);
		if (LoadingWidget)
		{
			LoadingWidget->AddToViewport();
		}
	}

	// 비동기 로딩
	StartAsyncLoad();
	*/
}

void ALoadingLevelManager::StartAsyncLoad()
{
	UE_LOG(LogTemp, Log, TEXT("[YiSan] Starting async load for level: %s"), *TargetLevelName.ToString());

	// 1. 에셋 로딩과 월드 파티션 확인을 동시에 시작
	LoadAssetsAndWorldPartition();

	// 2. 레벨 스트리밍을 시작
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = FName("OnLevelStreamed");
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = __LINE__;
	UGameplayStatics::LoadStreamLevel(this, TargetLevelName, true, false, LatentInfo);
}

void ALoadingLevelManager::LoadAssetsAndWorldPartition()
{
	// 월드 파티션 상태 확인 타이머를 시작합니다.
	GetWorldTimerManager().SetTimer(WorldPartitionCheckTimer, this, &ALoadingLevelManager::CheckWorldPartitionStatus, 0.1f, true);

	// 추가로 로드할 에셋 목록 (필요에 따라 추가)
	const TArray<FSoftObjectPath> AssetsToLoad = {
		// FSoftObjectPath(TEXT("/Game/Path/To/Your/Asset.Asset"))
	};

	if (AssetsToLoad.Num() > 0)
	{
		FStreamableManager& StreamMgr = UAssetManager::GetStreamableManager();
		StreamMgr.RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateUObject(this, &ALoadingLevelManager::OnAssetsLoaded));
	}
	else
	{
		// 로드할 추가 에셋이 없으면 바로 완료 처리
		bAssetsLoaded = true;
	}
}

void ALoadingLevelManager::OnLevelStreamed()
{
	UE_LOG(LogTemp, Log, TEXT("[YiSan] Level streaming finished."));
	bLevelStreamed = true;
	TryTransition();
}

void ALoadingLevelManager::OnAssetsLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("[YiSan] Asset loading finished."));
	bAssetsLoaded = true;
	TryTransition();
}

void ALoadingLevelManager::CheckWorldPartitionStatus()
{
	if (UWorldPartitionSubsystem* WP = GetWorld()->GetSubsystem<UWorldPartitionSubsystem>())
	{
		if (WP->IsStreamingCompleted(nullptr))
		{
			UE_LOG(LogTemp, Log, TEXT("[YiSan] World Partition streaming completed."));
			bWorldPartitionLoaded = true;
			GetWorldTimerManager().ClearTimer(WorldPartitionCheckTimer);
			TryTransition();
		}
	}
	else
	{
		// 월드 파티션이 없는 맵이면, 즉시 완료
		UE_LOG(LogTemp, Log, TEXT("[YiSan] No World Partition subsystem found. Assuming complete."));
		bWorldPartitionLoaded = true;
		GetWorldTimerManager().ClearTimer(WorldPartitionCheckTimer);
		TryTransition();
	}
}

void ALoadingLevelManager::TryTransition()
{
	// 모든 로딩(이 완료되었는지 확인
	if (bLevelStreamed && bAssetsLoaded && bWorldPartitionLoaded)
	{
		TransitionToMainLevel();
	}
}

void ALoadingLevelManager::TransitionToMainLevel()
{
	// 한 번만 실행
	if (GetWorldTimerManager().IsTimerActive(WorldPartitionCheckTimer))
	{
		GetWorldTimerManager().ClearTimer(WorldPartitionCheckTimer);
	} else {
        // 이미 전환이 시작되었다면 중복 실행을 막습니다.
        if(!PrimaryActorTick.bCanEverTick) return;
    }
    PrimaryActorTick.bCanEverTick = false;

	UE_LOG(LogTemp, Log, TEXT("[YiSan] All loading complete. Transitioning to level: %s"), *TargetLevelName.ToString());

	UGameplayStatics::OpenLevel(this, TargetLevelName);
}