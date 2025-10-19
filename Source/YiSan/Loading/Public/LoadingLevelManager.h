#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LoadingLevelManager.generated.h"

UCLASS()
class YISAN_API ALoadingLevelManager : public AActor
{
	GENERATED_BODY()

public:
	ALoadingLevelManager();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Loading")
	FName TargetLevelName = TEXT("MainLevel_WP");

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<class UUserWidget> LoadingWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingWidget;

	// 로딩 진행 상태 플래그
	bool bAssetsLoaded = false;
	bool bWorldPartitionLoaded = false;
	bool bLevelStreamed = false;

	FTimerHandle WorldPartitionCheckTimer;

	// 비동기 로딩 시작
	void StartAsyncLoad();

	// 에셋/월드파티션 로딩 시작
	void LoadAssetsAndWorldPartition();

	// 모든 로딩이 완료되었는지 확인하고 레벨 전환 시도
	void TryTransition();

	// 최종 레벨 전환
	void TransitionToMainLevel();

	// 콜백 함수들
	UFUNCTION()
	void OnAssetsLoaded();

	UFUNCTION()
	void OnLevelStreamed();

	UFUNCTION()
	void CheckWorldPartitionStatus();
};