// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "YiSanLoading.generated.h"

class SWidget;
class UUserWidget;
class ULoadingScreenWidget;

/**
 * 자동으로 맵 전환을 감지하여 로딩 UI를 표시하는 서브시스템
 */
UCLASS()
class YISAN_API UYiSanLoading : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_SUBSYSTEM_GETTER_INLINE(UYiSanLoading);
	
	// === 서브시스템 라이프사이클 ===
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// === Step별 함수 ===
	// Step 1: ServerTravel이 시작될 때 자동으로 호출됨 (PreLoadMap)
	void Step1_OnPreLoadMap(const FString& MapName);
	
	// Step 2: 맵 로드(OpenLevel/ServerTravel)가 완료된 직후 호출되는 콜백
	void Step2_OnPostLoadMap(UWorld* LoadedWorld);
	
	// Step 3: 모든 스트리밍(WP, 텍스처, 레벨 인스턴스)이 완료된 후 호출됨
	void Step3_TransitionToTarget();

	// === 리소스 체크 함수 ===
	// 스트리밍 및 레벨 인스턴스 준비 상태를 주기적으로 폴링
	void Poll_StreamingAndInstancesReady();

	// === UI 관련 ===
	// 로딩 UI를 뷰포트에 표시
	void ShowLoadingUI(TSubclassOf<UUserWidget> InLoadingWidgetClass);
	
	// 로딩 UI를 뷰포트에서 제거
	void HideLoadingUI();
	
	// 로딩 UI의 진행률과 상태 텍스트를 업데이트
	void UpdateLoadingUIProgress(float ProgressPercentage, const FText& StatusText, 
		float WorldPartitionProgress, float TextureProgress, float LevelInstanceProgress);
	
	// 로딩 위젯 클래스 (블루프린트 등에서 설정)
	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	// === 로딩 상태 ===
	FName TargetLevelName = "MainMap_WP";

	// 타임아웃 체크를 위한 시작 시간
	double ResourceCheckStartTime = 0.0;
	
	// 텍스처 스트리밍 추적 변수
	double TextureStreamingStartTime = 0.0;
	bool bInitialTextureStreamingComplete = false;

	// GameViewport에 추가한 슬레이트 위젯 레퍼런스 보관용
	TSharedPtr<SWidget> LoadingWidgetHolder; 
	
	// 생성된 UUserWidget 인스턴스 보관용 (GC 방지 및 접근)
	TWeakObjectPtr<UUserWidget> LoadingWidgetObject;
	
	// 스트리밍 완료 폴링 타이머 핸들
	FTimerHandle PollingStreamingTimerHandle;

private:
	// 델리게이트 핸들 (정리용)
	FDelegateHandle PreLoadMapHandle;
	FDelegateHandle PostLoadMapHandle;
};