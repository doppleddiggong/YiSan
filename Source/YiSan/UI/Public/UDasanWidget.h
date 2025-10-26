// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDasanState.h"
#include "UDasanWidget.generated.h"

/**
 * 다산 NPC 머리 위에 표시되는 상태 표시 위젯
 */
UCLASS()
class YISAN_API UDasanWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 다산 액터 참조 초기화
	void InitWidget(class ADasanActor* InDasanActor);

	// 다산 상태 업데이트 (메인 상태 + 서브 상태)
	void UpdateDasanState(EDasanState MainState, ETourState TourState, EExplainState ExplainState, EAnswerState AnswerState);

private:
	// 상태에 따른 메시지 텍스트 반환
	FString GetStateMessage(EDasanState MainState, ETourState TourState, EExplainState ExplainState, EAnswerState AnswerState) const;

public:
	// 이미지의 경우에는 Blueprint에서 상태 제어가 가능하게 빼준다
	UPROPERTY(meta = (BindWidget))
	class UImage* Image_Icon;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Msg;

private:
	UPROPERTY()
	TObjectPtr<class ADasanActor> DasanActor;
};
