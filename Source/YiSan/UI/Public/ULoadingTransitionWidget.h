// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ULoadingTransitionWidget.generated.h"

/**
 * 로딩 화면 위젯 - 각 컴포넌트별 진행률을 시각화합니다.
 */
UCLASS()
class YISAN_API ULoadingTransitionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 전체 로딩 진행률 업데이트
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void UpdateProgress(float Progress );

protected:
	virtual void NativeConstruct() override;

	// 전체 진행률 바
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressBar;

	// 전체 진행률 텍스트 (예: 75%)
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ProgressText;

	// 현재 상태 텍스트 (예: "텍스처 로딩 중...")
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* StatusText;

	// 로딩 팁 텍스트 (선택사항)
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* LoadingTipText;

	// 배경 이미지 (선택사항)
	UPROPERTY(meta = (BindWidget))
	class UImage* BackgroundImage;
};