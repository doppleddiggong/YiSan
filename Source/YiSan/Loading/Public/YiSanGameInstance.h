// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "YiSanGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class YISAN_API UYiSanGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UYiSanGameInstance();
#pragma region map

	/**
	 * 로딩 레벨을 거쳐서 타겟 레벨로 전환
	 * @param InTargetLevelName 최종 목적지 레벨 이름
	 */
	UFUNCTION(BlueprintCallable, Category = "Level")
	void LoadLevelWithLoadingScreen(FName InTargetLevelName);

protected:
	virtual void Init() override;
	
public:
	/** 로딩 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	/** 로딩 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingWidget;

	/** 최종 목적지 레벨 */
	FName TargetLevelName;

	/** 로딩 완료 플래그 */
	bool bLevelLoaded = false;
	bool bAssetsLoaded = false;
	bool bWorldPartitionReady = false;
	bool bIsLoadingLevel = false;

	/** 타이머 핸들 */
	FTimerHandle WorldPartitionCheckTimer;
	FTimerHandle ResourceCheckTimer;

	// ==================== 단계별 처리 ====================

	/** Step 1: 로딩 레벨로 이동 */
	void Step1_MoveToLoadingLevel();
	void OnLoadingMapReady();

	/** Step 2: 로딩 레벨에서 타겟 레벨 로드 시작 */
	UFUNCTION()
	void Step2_StartLoadingTargetLevel();
	void OnTargetLevelReady();

	/** Step 3: 레벨 로드 완료 콜백 */
	UFUNCTION()
	void Step3_OnLevelLoaded();

	/** Step 4: 리소스 스트리밍 체크 */
	void Step4_CheckResources();
	void PeriodicResourceCheck();

	/** Step 5: 모든 준비 완료, 타겟 레벨로 전환 */
	void Step5_TransitionToTarget();

	/** 리소스 로딩 상태 확인 */
	bool CheckTextureStreaming();
	bool CheckShaderCompilation();
	bool CheckWorldPartition();

	/** 로딩 UI 표시/숨김 */
	void ShowLoadingScreen();
	void HideLoadingScreen();
	
	void FinalHideLoadingScreen();

#pragma endregion map


public:
	//세션의 모든 처리 진행 객체
	IOnlineSessionPtr sessionInterface;

	//세션 생성 
	//세션 생성 함수
	UFUNCTION(BlueprintCallable)
	void CreateMySession(FString displayName, int32 playerCount);
	//세션 생성 완료 함수
	void OnCreateSessionComplete(FName sessionName, bool success);

	//세션 조회 
	//세션 조회할 때 사용하는 객체
	TSharedPtr<FOnlineSessionSearch> sessionSearch;
	//세션 조회 함수
	UFUNCTION(BlueprintCallable)
	void FindOtherSession();
	//세션 조회 완료 함수
	void OnFindSessionComplete(bool success);
	
	//세션 참여 
	//세션 참여 함수
	UFUNCTION(BlueprintCallable)
	void JoinOtherSession(int32 sessionIndex);
	//세션 참여 완료 함수
	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);

	
};
