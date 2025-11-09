// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

/**
 * @file UDasanWidget.h
 * @brief UDasanWidget 클래스를 선언합니다.
 */
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDasanState.h"
#include "UDasanWidget.generated.h"

/**
 * @brief 다산(Dasan) NPC의 머리 위에 표시되어 현재 상태를 알려주는 UI 위젯입니다.
 * @details 투어, 질의응답 등 NPC의 주 상태와 서브 상태에 따라 다른 아이콘과 메시지를 표시하여 플레이어에게 시각적인 피드백을 제공합니다.
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
	void UpdateDasanState(EDasanState MainState, ETourState TourState, EAnswerState AnswerState,
		const FString& BuildingName = TEXT(""), const FString& PlayerName = TEXT(""),
		int32 CurrentPlayers = 0, int32 MaxPlayers = 0);

	// 설명 다이얼로그 표시 (3초 후 자동 숨김)
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void ShowExplainDialog(const FString& ExplainText);
	// 설명 다이얼로그 숨김
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void HideExplainDialog();

private:
	// 상태에 따른 메시지 텍스트 반환
	FString GetStateMessage(EDasanState MainState, ETourState TourState, EAnswerState AnswerState,
		const FString& BuildingName, const FString& PlayerName, int32 CurrentPlayers, int32 MaxPlayers) const;
	FString GetResourcePath(EDasanState MainState, ETourState TourState, EAnswerState AnswerState) const;

	
public:
	// 이미지의 경우에는 Blueprint에서 상태 제어가 가능하게 빼준다
	UPROPERTY(meta = (BindWidget))
	class UImage* Image_Icon;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Msg;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Explain;

	UPROPERTY(meta = (BindWidget))
	class UBorder* Border_Explain;

private:
	UPROPERTY()
	TObjectPtr<class ADasanActor> DasanActor;

	// 설명 다이얼로그 자동 숨김 타이머
	FTimerHandle ExplainDialogTimerHandle;
};
