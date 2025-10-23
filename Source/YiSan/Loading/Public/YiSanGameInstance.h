// Copyright (c) 2025 Doppleddiggong. 모든 권리 보유함.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "YiSanGameInstance.generated.h"

class SWidget;
class UUserWidget;

UCLASS()
class YISAN_API UYiSanGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	
	// Step별 함수임
	// Step 1: 로딩 UI를 표시하고 타겟 레벨 로드(OpenLevel 또는 ServerTravel)를 시작함.
	void Step1_StartLoadingTargetLevel();

	// Step 2: 맵 로드(OpenLevel/ServerTravel)가 완료된 직후 호출되는 콜백임.
	void Step2_OnPostLoadMap(UWorld* LoadedWorld);
	
	// Step 3: 모든 스트리밍(WP, 텍스처, 레벨 인스턴스)이 완료된 후 호출됨.
	void Step3_TransitionToTarget();

	// 리소스 체크 함수임
	// 스트리밍 및 레벨 인스턴스 준비 상태를 주기적으로 폴링함.
	void Poll_StreamingAndInstancesReady();

	// UI 관련
	// 로딩 UI를 뷰포트에 표시함.
	void ShowLoadingUI(TSubclassOf<UUserWidget> LoadingWidgetClass);
	
	// 로딩 UI를 뷰포트에서 제거함.
	void HideLoadingUI();
	
	// 로딩 UI의 진행률과 상태 텍스트를 업데이트함 (현재는 로그만 출력).
	void UpdateLoadingUIProgress(float ProgressPercentage, const FText& StatusText);

	
	// 로딩 위젯 클래스 (블루프린트 등에서 설정).
	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	// 로딩 상태임
	FName TargetLevelName = "Main_MapWP";

	// 타임아웃 체크를 위한 시작 시간 (현재 로직에서는 사용되지 않으나 추후 확장용으로 둠).
	double ResourceCheckStartTime = 0.0;

private:
	// GameViewport에 추가한 슬레이트 위젯 레퍼런스 보관용임.
	TSharedPtr<SWidget> LoadingWidgetHolder; 
	
	// 생성된 UUserWidget 인스턴스 보관용임 (GC 방지 및 접근).
	TWeakObjectPtr<UUserWidget> LoadingWidgetObject;
	
	// 스트리밍 완료 폴링 타이머 핸들임.
	FTimerHandle PollingStreamingTimerHandle;
};