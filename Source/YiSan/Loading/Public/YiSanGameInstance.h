// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "YiSanGameInstance.generated.h"

UCLASS()
class YISAN_API UYiSanGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	
	// Step별 함수들
	void UpdateLoadingUIProgress(float ProgressPercentage);

	void Step2_StartLoadingTargetLevel();

	void Step5_TransitionToTarget();
    
	// 수정: 레벨 로드 완료 콜백 추가
	void OnPostLoadMap(UWorld* LoadedWorld);

	// 리소스 체크 함수들
	void Poll_StreamingAndInstancesReady();
	bool CheckTextureStreaming();
	bool CheckWorldPartition();

	// UI 관련
	void ShowLoadingUI(TSubclassOf<UUserWidget> LoadingWidgetClass);
	void HideLoadingUI();

	
	// 로딩 위젯
	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	UPROPERTY()
	UUserWidget* LoadingWidget = nullptr;

	// 로딩 상태
	FName TargetLevelName = "Main_MapWP";
	bool bIsLoadingLevel = false;
	bool bLevelLoaded = false;

	// 타이머
	FTimerHandle ResourceCheckTimer;
    
	// 수정: 타임아웃 체크를 위한 시작 시간 추가
	double ResourceCheckStartTime = 0.0;

	TSharedPtr<SWidget> LoadingWidgetHolder; // GameViewport에 추가한 위젯 보관
	TWeakObjectPtr<UUserWidget> LoadingWidgetObject; // UUserWidget 인스턴스 보관 (GC 안전성)
	FTimerHandle PollingStreamingTimerHandle; // 스트리밍 완료 폴링 타이머
};