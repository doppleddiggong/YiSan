// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimpleLoadingWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 간단한 로딩 화면 위젯 - 전체 진행률만 표시
 */
UCLASS()
class YISAN_API USimpleLoadingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 진행률 업데이트
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void UpdateProgress(float Progress, const FText& StatusText);

protected:
	virtual void NativeConstruct() override;

	// 진행률 바
	UPROPERTY(meta = (BindWidget))
	UProgressBar* LoadingProgressBar;

	// 진행률 텍스트
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ProgressText;

	// 상태 텍스트
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;

private:
	FText GetPercentageText(float Progress) const;
};