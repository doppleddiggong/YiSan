// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

/**
 * @file ULoadingTransitionManager.h
 * @brief ULoadingTransitionManager 클래스를 선언합니다.
 */
#include "CoreMinimal.h"
#include "Macro.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ULoadingTransitionManager.generated.h"

/**
 * @brief 레벨 전환 시 전체 화면 로딩 위젯의 표시와 숨김을 관리하는 LocalPlayer 서브시스템입니다.
 * @details 맵 로드 시작 및 완료 이벤트를 감지하여 로딩 위젯을 자동으로 표시하고, 최소 노출 시간을 보장한 후 부드럽게 숨기는 역할을 합니다.
 */
UCLASS()
class YISAN_API ULoadingTransitionManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_LOCALPLAYER_SUBSYSTEM_GETTER_INLINE(ULoadingTransitionManager);

	ULoadingTransitionManager();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	/// @brief 로딩 화면을 표시합니다.
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void ShowLoadingScreen();

	/// @brief 로딩 화면을 제거합니다.
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void HideLoadingScreen();

private:
	void EnsureWidgetForWorld(UWorld* World);

	void FinalizeHide();

	void HandlePreLoadMap(const struct FWorldContext& WorldContext, const FString& MapName);
	void HandlePostLoadMap(UWorld* LoadedWorld);
	bool DoesWorldBelongToLocalPlayer(const UWorld* World) const;

	/// @brief 로딩 화면 제거를 지연 처리하기 위한 최소 노출 시간(초)
	static constexpr double MinVisibleDurationSeconds = 1.5;

	/// @brief 완료 이후 페이드아웃 전 유지 시간(초)
	static constexpr double HoldAfterCompletionSeconds = 0.35;
	
private:
	UPROPERTY()
	TSubclassOf<class ULoadingTransitionWidget> TransitionWidgetClass;

	UPROPERTY()
	TObjectPtr<class ULoadingTransitionWidget> TransitionWidget;

	double LastShowTimestamp = 0.0;
	float LatestReportedProgress = 0.0f;
	bool bHideRequested = false;

	FTimerHandle HideTimerHandle;

	FDelegateHandle PreLoadMapHandle;
	FDelegateHandle PostLoadMapHandle;

	bool bIsShowing = false;
};
