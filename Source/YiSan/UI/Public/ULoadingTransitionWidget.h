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
	void AddToGameViewport(int32 ZOrder);
	void RefreshLoadingTip();
	
protected:
	// 로딩 팁 텍스트
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* LoadingTipText;
	
private:
	TArray<FString> LoadingTips = {
		TEXT("[TIP] WASD 키로 캐릭터를 이동할 수 있습니다."),
		TEXT("[TIP] 마우스로 카메라 시점을 변경할 수 있습니다."),
		TEXT("[TIP] 최적의 게임 경험을 위해 그래픽 설정을 조정하세요."),
		TEXT("[TIP] 초기 로딩은 셰이더 컴파일로 인해 시간이 걸릴 수 있습니다."),

		TEXT("[TIP] 정조 이산은 백성을 위한 개혁 군주였어요."),
		TEXT("[TIP] 정약용은 다산이라 불리며 실학을 집대성했습니다."),
		TEXT("[TIP] 화성은 조선 최고의 계획도시로 평가받습니다."),
		TEXT("[TIP] 정조는 아버지 사도세자의 명예 회복을 위해 노력했습니다."),
		TEXT("[TIP] 화성 행궁은 정조가 직접 설계에 관여한 건축물입니다."),
		TEXT("[TIP] 다산 정약용은 거중기를 만들어 축성에 기여했습니다."),
		TEXT("[TIP] 화성 성곽은 유네스코 세계문화유산입니다."),
		TEXT("[TIP] 정조는 규장각을 세워 학문과 인재를 중시했습니다."),
		TEXT("[TIP] 정약용은 목민심서를 집필하여 백성 다스리는 도리를 강조했습니다."),
		TEXT("[TIP] 화성 축성은 단 2년 반 만에 완성되었습니다.")
	};
};