// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ULoadginCircle.generated.h"

/// @brief 로딩 카운트 변경 시 호출되는 델리게이트입니다.
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadingCountChanged, int32 /*NewCount*/);

/**
 * 네트워크 대기 상태를 표시하는 로딩 서클 위젯입니다.
 * Count 기반으로 여러 작업의 로딩 상태를 관리합니다.
 */
UCLASS()
class YISAN_API ULoadginCircle : public UUserWidget
{
	GENERATED_BODY()

public:
	/// @brief 위젯 생성 시 초기화 작업을 수행합니다.
	virtual void NativeConstruct() override;

	/// @brief 위젯 파괴 시 타이머와 델리게이트를 정리합니다.
	virtual void NativeDestruct() override;

	/// @brief 매 프레임 상태 표시를 갱신합니다.
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/// @brief 로딩 카운트를 증가시키고 로딩 서클을 표시합니다.
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void ShowLoading();

	/// @brief 로딩 카운트를 감소시키고, 카운트가 0이 되면 로딩 서클을 숨깁니다.
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void HideLoading();

	/// @brief 현재 로딩 카운트를 반환합니다.
	UFUNCTION(BlueprintPure, Category = "Loading")
	int32 GetLoadingCount() const { return LoadingCount; }

	/// @brief 위젯을 Game Viewport에 추가하여 레벨 전환 시에도 유지되도록 합니다.
	/// @param ZOrder 뷰포트에서의 레이어 순서 (높을수록 위에 표시됨)
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void AddToGameViewport(int32 ZOrder = 1000);

	/// @brief 로딩 카운트가 변경될 때 호출되는 델리게이트입니다.
	FOnLoadingCountChanged OnLoadingCountChanged;

private:
	/// @brief 로딩 스피너 회전 애니메이션을 갱신합니다.
	void UpdateLoadingSpinner(float DeltaTime);

	/// @brief 로딩 카운트에 따라 위젯 가시성을 업데이트합니다.
	void UpdateVisibility();

private:
	/// @brief 현재 로딩 중인 작업의 개수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loading", meta = (AllowPrivateAccess = "true"))
	int32 LoadingCount = 0;

public:
	/// @brief 네트워크 대기 상태를 나타내는 로딩 스피너 이미지입니다.
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "State|Network")
	class UImage* LoadingSpinner = nullptr;

	/// @brief 로딩 스피너의 회전 속도(초당 각도)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State|Network")
	float SpinnerRotationSpeed = 90.0f;
};
