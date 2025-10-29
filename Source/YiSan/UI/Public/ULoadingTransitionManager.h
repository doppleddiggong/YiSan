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

	protected:
	/// @brief 현재 월드에 맞는 위젯이 없으면 생성하여 GameViewport에 추가합니다.
	void EnsureWidgetForWorld(UWorld* World);

private:
	UPROPERTY()
	TSubclassOf<class ULoadingTransitionWidget> TransitionWidgetClass;

	UPROPERTY()
	TObjectPtr<class ULoadingTransitionWidget> TransitionWidget;
};
