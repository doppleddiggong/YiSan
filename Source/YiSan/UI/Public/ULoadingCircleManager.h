// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.
#pragma once

/**
 * @file ULoadingCircleManager.h
 * @brief ULoadingCircleManager 클래스를 선언합니다.
 */
#include "CoreMinimal.h"
#include "Macro.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ULoadingCircleManager.generated.h"

/**
 * 전역 로딩 서클 관리자
 * LocalPlayerSubsystem으로 구현되어 레벨 전환 시에도 유지됩니다.
 */
UCLASS()
class YISAN_API ULoadingCircleManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_LOCALPLAYER_SUBSYSTEM_GETTER_INLINE(ULoadingCircleManager);

	ULoadingCircleManager();

	/// @brief 로딩 카운트를 증가시킵니다.
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void Show();

	/// @brief 로딩 카운트를 감소시킵니다.
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void Hide();

	/// @brief 현재 로딩 카운트를 반환합니다.
	UFUNCTION(BlueprintPure, Category = "Loading")
	int32 GetLoadingCount() const;

protected:
	/// @brief 현재 월드에 위젯이 없으면 생성하고 Game Viewport에 부착합니다.
	void EnsureWidgetForWorld(UWorld* World);

protected:
	UPROPERTY()
	TSubclassOf<class ULoadginCircle> CircleWidgetClass;

	UPROPERTY()
	TObjectPtr<class ULoadginCircle> CircleWidget;

	/// @brief Manager가 관리하는 로딩 카운트 (위젯 재생성 시에도 유지됨)
	int32 LoadingCount = 0;
};
