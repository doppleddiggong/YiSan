// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingScreenWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UVerticalBox;
class UImage;

/**
 * 로딩 화면 위젯 - 각 컴포넌트별 진행률을 시각화합니다.
 */
UCLASS()
class YISAN_API ULoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 전체 로딩 진행률 업데이트
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void UpdateOverallProgress(float Progress, const FText& StatusText);

	// 개별 컴포넌트 진행률 업데이트
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void UpdateComponentProgress(float WorldPartitionProgress, float TextureProgress, float LevelInstanceProgress);

	// 컴포넌트별 상태 업데이트 (완료/진행중/대기)
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void UpdateComponentStatus(const FText& WPStatus, const FText& TextureStatus, const FText& LevelInstanceStatus);

protected:
	virtual void NativeConstruct() override;

	// 메인 UI 컴포넌트들 (블루프린트에서 바인딩)
	
	// 전체 진행률 바
	UPROPERTY(meta = (BindWidget))
	UProgressBar* OverallProgressBar;

	// 전체 진행률 텍스트 (예: 75%)
	UPROPERTY(meta = (BindWidget))
	UTextBlock* OverallProgressText;

	// 현재 상태 텍스트 (예: "텍스처 로딩 중...")
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;

	// 월드 파티션 진행률 바
	UPROPERTY(meta = (BindWidget))
	UProgressBar* WorldPartitionProgressBar;

	// 월드 파티션 진행률 텍스트
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WorldPartitionProgressText;

	// 월드 파티션 상태 텍스트
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WorldPartitionStatusText;

	// 텍스처 진행률 바
	UPROPERTY(meta = (BindWidget))
	UProgressBar* TextureProgressBar;

	// 텍스처 진행률 텍스트
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextureProgressText;

	// 텍스처 상태 텍스트
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextureStatusText;

	// 레벨 인스턴스 진행률 바
	UPROPERTY(meta = (BindWidget))
	UProgressBar* LevelInstanceProgressBar;

	// 레벨 인스턴스 진행률 텍스트
	UPROPERTY(meta = (BindWidget))
	UTextBlock* LevelInstanceProgressText;

	// 레벨 인스턴스 상태 텍스트
	UPROPERTY(meta = (BindWidget))
	UTextBlock* LevelInstanceStatusText;

	// 배경 이미지 (선택사항)
	UPROPERTY(meta = (BindWidget))
	UImage* BackgroundImage;

	// 로딩 팁 텍스트 (선택사항)
	UPROPERTY(meta = (BindWidget))
	UTextBlock* LoadingTipText;

private:
	// 진행률을 퍼센트 텍스트로 변환
	FText GetPercentageText(float Progress) const;

	// 진행률에 따른 색상 반환 (0.0 = 빨강, 0.5 = 노랑, 1.0 = 초록)
	FLinearColor GetProgressColor(float Progress) const;
};