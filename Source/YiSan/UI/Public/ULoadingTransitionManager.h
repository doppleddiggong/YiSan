// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Macro.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ULoadingTransitionManager.generated.h"

UCLASS()
class YISAN_API ULoadingTransitionManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_LOCALPLAYER_SUBSYSTEM_GETTER_INLINE(ULoadingTransitionManager);

	ULoadingTransitionManager();

	/// @brief 로딩 화면을 표시합니다.
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void ShowLoadingScreen();

	/// @brief 로딩 화면의 진행률을 업데이트합니다.
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void UpdateLoadingProgress(float Progress);

	/// @brief 로딩 화면을 제거합니다.
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void HideLoadingScreen();



private:
	/// @brief 현재 월드에 맞는 위젯이 없으면 생성하여 GameViewport에 추가합니다.
	void EnsureWidgetForWorld(UWorld* World);
	
	void FinalizeHide();

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
};
