// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#include "LoadingScreenWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"

void ULoadingScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기 상태 설정
	if (OverallProgressBar)
	{
		OverallProgressBar->SetPercent(0.0f);
	}

	if (WorldPartitionProgressBar)
	{
		WorldPartitionProgressBar->SetPercent(0.0f);
	}

	if (TextureProgressBar)
	{
		TextureProgressBar->SetPercent(0.0f);
	}

	if (LevelInstanceProgressBar)
	{
		LevelInstanceProgressBar->SetPercent(0.0f);
	}

	// 초기 상태 텍스트
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("로딩을 준비하는 중...")));
	}

	if (WorldPartitionStatusText)
	{
		WorldPartitionStatusText->SetText(FText::FromString(TEXT("대기 중")));
	}

	if (TextureStatusText)
	{
		TextureStatusText->SetText(FText::FromString(TEXT("대기 중")));
	}

	if (LevelInstanceStatusText)
	{
		LevelInstanceStatusText->SetText(FText::FromString(TEXT("대기 중")));
	}

	// 로딩 팁 설정 (선택사항)
	if (LoadingTipText)
	{
		TArray<FString> LoadingTips = {
			TEXT("팁: WASD 키로 캐릭터를 이동할 수 있습니다."),
			TEXT("팁: 마우스로 카메라 시점을 변경할 수 있습니다."),
			TEXT("팁: 최적의 게임 경험을 위해 그래픽 설정을 조정하세요."),
			TEXT("팁: 초기 로딩은 셰이더 컴파일로 인해 시간이 걸릴 수 있습니다."),
		};

		int32 RandomIndex = FMath::RandRange(0, LoadingTips.Num() - 1);
		LoadingTipText->SetText(FText::FromString(LoadingTips[RandomIndex]));
	}
}

void ULoadingScreenWidget::UpdateOverallProgress(float Progress, const FText& StatusTextValue)
{
	Progress = FMath::Clamp(Progress, 0.0f, 1.0f);

	// 전체 진행률 바 업데이트
	if (OverallProgressBar)
	{
		OverallProgressBar->SetPercent(Progress);
		OverallProgressBar->SetFillColorAndOpacity(GetProgressColor(Progress));
	}

	// 전체 진행률 텍스트 업데이트
	if (OverallProgressText)
	{
		OverallProgressText->SetText(GetPercentageText(Progress));
	}

	// 상태 텍스트 업데이트
	if (StatusText)
	{
		StatusText->SetText(StatusTextValue);
	}
}

void ULoadingScreenWidget::UpdateComponentProgress(float WorldPartitionProgress, float TextureProgress, float LevelInstanceProgress)
{
	// 월드 파티션 진행률
	if (WorldPartitionProgressBar)
	{
		WorldPartitionProgressBar->SetPercent(WorldPartitionProgress);
		WorldPartitionProgressBar->SetFillColorAndOpacity(GetProgressColor(WorldPartitionProgress));
	}

	if (WorldPartitionProgressText)
	{
		WorldPartitionProgressText->SetText(GetPercentageText(WorldPartitionProgress));
	}

	// 텍스처 진행률
	if (TextureProgressBar)
	{
		TextureProgressBar->SetPercent(TextureProgress);
		TextureProgressBar->SetFillColorAndOpacity(GetProgressColor(TextureProgress));
	}

	if (TextureProgressText)
	{
		TextureProgressText->SetText(GetPercentageText(TextureProgress));
	}

	// 레벨 인스턴스 진행률
	if (LevelInstanceProgressBar)
	{
		LevelInstanceProgressBar->SetPercent(LevelInstanceProgress);
		LevelInstanceProgressBar->SetFillColorAndOpacity(GetProgressColor(LevelInstanceProgress));
	}

	if (LevelInstanceProgressText)
	{
		LevelInstanceProgressText->SetText(GetPercentageText(LevelInstanceProgress));
	}
}

void ULoadingScreenWidget::UpdateComponentStatus(const FText& WPStatus, const FText& TextureStatus, const FText& LevelInstanceStatus)
{
	if (WorldPartitionStatusText)
	{
		WorldPartitionStatusText->SetText(WPStatus);
	}

	if (TextureStatusText)
	{
		TextureStatusText->SetText(TextureStatus);
	}

	if (LevelInstanceStatusText)
	{
		LevelInstanceStatusText->SetText(LevelInstanceStatus);
	}
}

FText ULoadingScreenWidget::GetPercentageText(float Progress) const
{
	int32 Percentage = FMath::RoundToInt(Progress * 100.0f);
	return FText::FromString(FString::Printf(TEXT("%d%%"), Percentage));
}

FLinearColor ULoadingScreenWidget::GetProgressColor(float Progress) const
{
	// 진행률에 따라 색상 변경
	// 0.0 ~ 0.3: 빨강 -> 주황
	// 0.3 ~ 0.7: 주황 -> 노랑
	// 0.7 ~ 1.0: 노랑 -> 초록

	if (Progress < 0.3f)
	{
		// 빨강 -> 주황
		float Alpha = Progress / 0.3f;
		return FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.2f, 0.2f), FLinearColor(1.0f, 0.6f, 0.2f), Alpha);
	}
	else if (Progress < 0.7f)
	{
		// 주황 -> 노랑
		float Alpha = (Progress - 0.3f) / 0.4f;
		return FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.6f, 0.2f), FLinearColor(1.0f, 1.0f, 0.2f), Alpha);
	}
	else
	{
		// 노랑 -> 초록
		float Alpha = (Progress - 0.7f) / 0.3f;
		return FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 1.0f, 0.2f), FLinearColor(0.2f, 1.0f, 0.2f), Alpha);
	}
}

