// Copyright (c) 2025 Doppleddiggong. All rights reserved.

#include "ULoadingTransitionWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void ULoadingTransitionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기 상태 설정
	ProgressBar->SetPercent(0.0f);
	// 초기 상태 텍스트
	StatusText->SetText(FText::FromString(TEXT("로딩을 준비하는 중...")));

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

void ULoadingTransitionWidget::UpdateProgress(float Progress)
{
	ProgressBar->SetPercent(Progress);

	int32 Percentage = FMath::RoundToInt(Progress * 100.0f);
	ProgressText->SetText( FText::FromString(FString::Printf(TEXT("%d%%"), Percentage)) );

	StatusText->SetText(FText::FromString("로딩 STATUS 테스트"));
}