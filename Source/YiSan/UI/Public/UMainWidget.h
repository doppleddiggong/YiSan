// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBuildingType.h"
#include "Blueprint/UserWidget.h"
#include "NetworkData.h"
#include "UMainWidget.generated.h"

/// @file UMainWidget.h
/// @brief 음성 상호작용과 건물 안내 UI를 제공하는 메인 위젯을 선언합니다.
/// @brief 채팅, 팝업, 네트워크 응답을 중재하는 게임 메인 UI 위젯입니다.

UCLASS()
class YISAN_API UMainWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /// @brief 채팅 박스의 표시 상태를 전환합니다.
    void ToggleChatBox();

protected:
    /// @brief 위젯 초기화와 브로드캐스트 구독을 수행합니다.
    virtual void NativeConstruct() override;

    /// @brief 드래그 이동을 지원하기 위해 마우스 다운 이벤트를 처리합니다.
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    /// @brief 채팅 입력이 커밋될 때 메시지를 전송합니다.
    UFUNCTION()
    void OnMessageComitted(const FText& Text, ETextCommit::Type CommitMethod);

private:
    /// @brief 입력된 문자열을 네트워크로 전송합니다.
    void SendChatMessage(const FString& InMsg);

    /// @brief 현재 플레이어 위치 기반 GPT 컨텍스트를 구성합니다.
    FGPTContext BuildSpatialContext() const;

    /// @brief ASK 응답을 수신해 UI를 갱신합니다.
    /// @param Response [in] 음성/텍스트 처리 결과입니다.
    /// @param bSuccess [in] 요청 성공 여부입니다.
    UFUNCTION()
    void OnResponseAsk(FResponseAsk& Response, bool bSuccess);

protected:
    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Chat")
    class UCanvasPanel* ChatBox;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Chat")
    class UEditableTextBox* InputText;

private:
    UPROPERTY()
    TObjectPtr<class UBroadcastManager> BroadcastManager;

public:
    /// @brief 메가 팝업의 표시 상태를 전환합니다.
    void ToggleMegaPopup();

    /// @brief 주변 건물 브로드캐스트를 수신해 UI를 업데이트합니다.
    /// @param InBuildingType [in] 감지된 건물 유형입니다.
    UFUNCTION()
    void OnNearBuildingBroadcast(EBuildingType InBuildingType);

    /// @brief 스몰 팝업 텍스트를 갱신하는 블루프린트 이벤트입니다.
    UFUNCTION(BlueprintImplementableEvent)
    void BPI_UpdateSmallPopupText(EBuildingType BuildingType);

    /// @brief 상세 정보 패널을 갱신하는 블루프린트 이벤트입니다.
    UFUNCTION(BlueprintImplementableEvent)
    void BPI_UpdateDetailedInfo(EBuildingType BuildingType);

    /// @brief 스몰 팝업이 배치되는 컨테이너 위젯입니다.
    UPROPERTY(meta = (BindWidget))
    class UUserWidget* SmallPopupCtn;

    /// @brief 메가 팝업이 배치되는 컨테이너 위젯입니다.
    UPROPERTY(meta = (BindWidget))
    class UUserWidget* MegaPopupCtn;
   

private:
    /// @brief 메가 팝업 표시 여부입니다.
    bool bIsMegaPopupVisible = false;

    /// @brief 최근 감지된 건물 유형입니다.
    EBuildingType CurNearBuildingType = EBuildingType::None;
};

